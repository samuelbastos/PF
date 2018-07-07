#include "Renderer.h"
#include <glm/gtc/matrix_access.hpp>
#include <stdlib.h>     /* srand, rand */
#include <iostream>
#include <algorithm>
#include <thread>
#include <chrono>         // std::chrono::seconds
#include <string>
#include "Reader.h"

#ifndef DEGREE_TO_RADIANS
#define DEGREE_TO_RADIANS(s) (s * (glm::pi<double>() / 180.0))
#endif

#define VOS 256					// VOLUME ORININAL SIZE
#define TD 32					// TILE DIMENSION

Renderer::Renderer()
{
	vr_outdated = true;
}

Renderer::~Renderer(){}

void Renderer::loadBrick(int brickID)
{
	if (!Model::getInstance()->getBrickVisibility(brickID))
	{
		auto newStoragePoint = Model::getInstance()->genNewStoragePoint();
		int positionInFile = Model::getInstance()->getPositionInFileById(brickID);
		auto brick = m_reader->readTileData(positionInFile);

		Model::getInstance()->setBrickPosition(brickID, newStoragePoint);
		Model::getInstance()->setBrickVisible(brickID);

		glsl_bricks_buffer->SetSubData(brick, (int)newStoragePoint.x,
			(int)newStoragePoint.y, (int)newStoragePoint.z, GL_RED, GL_UNSIGNED_BYTE);
	}
}

void Renderer::loadChilds(int brickID)
{
	auto childIDs = Model::getInstance()->getChilds(brickID);
	for (int id : childIDs)
		Renderer::loadBrick(id);
}

void Renderer::updateTreeCut(int brickID)
{
	auto childs = Model::getInstance()->getChilds(brickID);
	if (childs.size() != 0)
	{
		if (Model::getInstance()->getBrickVisibility(childs[0]))
		{
			for (int child : childs)
				updateTreeCut(child);
		}
		else
		{
			glm::vec3 brickCoord = Model::getInstance()->getBrickCoord(brickID);
			glm::vec3 eye = -(glm::vec3(ViewMatrix[3])) * glm::mat3(ViewMatrix);
			glm::vec3 dif = brickCoord - eye;
			float distance = sqrtf(dif.x*dif.x + dif.y*dif.y + dif.z*dif.z);
			if (distance < 300)
				loadChilds(brickID);
		}
	}
}

void Renderer::threadfunc()
{
	while (true)
	{
		std::this_thread::sleep_for(std::chrono::seconds(1));
		vr_outdated = true;
	}
}

void Renderer::init(int screenWidth, int screenHeight)
{
	m_reader = new DiskReader();

	m_volume_dim = glm::vec3((float)VOS, (float)VOS, (float)VOS);
	m_brick_dim  = glm::vec3((float)TD, (float)TD, (float)TD);
	vr_stepsize = 0.1f;

	glsl_bricks_buffer = new gl::Texture3D(VOS, VOS, VOS, TD, TD, TD);
	glsl_bricks_buffer->GenerateTexture(GL_LINEAR, GL_LINEAR, GL_CLAMP_TO_BORDER, GL_CLAMP_TO_BORDER, GL_CLAMP_TO_BORDER);

	glsl_bricks_buffer->SetStorage(GL_R32F);

	Renderer::loadBrick(0);
	auto tf = vr::ReadTransferFunction("testeTF.tf1d");
	glsl_transfer_function = tf->GenerateTexture_1D_RGBA();

	ProjectionMatrix = glm::mat4();
	ViewMatrix = glm::mat4();

	createRenderingPass();

	shader_rendering->Bind();
	shader_rendering->BindUniforms();
	shader_rendering->Unbind();
	gl::VAO::Unbind();
	gl::Shader::Unbind();

	resize(screenWidth, screenHeight);
	std::thread test(&Renderer::threadfunc, this);
	test.detach();
}

void Renderer::createRenderingPass()
{
	// Quad Screen
	const Vertex VERTICES[4] = {
		{ { -1, -1, 0 } },
		{ { -1, +1, 0 } },
		{ { +1, +1, 0 } },
		{ { +1, -1, 0 } } };

	// Triangulos
	const GLuint INDICES[6] = { 0, 2, 1, 0, 3, 2 };

	//Rendering Pass
	shader_rendering = new gl::Shader(
		"texture.vs",
		"texture.fs"
	);
	
	shader_rendering->Bind();
	shader_rendering->SetUniform("ViewMatrix", ViewMatrix);
	shader_rendering->SetUniformTexture3D("BricksBuffer", glsl_bricks_buffer->GetTextureID(), 0);
	shader_rendering->SetUniformTexture1D("TexTransferFunc", glsl_transfer_function->GetTextureID(), 1);
	shader_rendering->SetUniform("VolumeDimension", m_volume_dim);
	shader_rendering->SetUniform("BrickDimension", m_brick_dim);
	shader_rendering->SetUniform("StepSize", vr_stepsize);

	glUniform3fv(glGetUniformLocation(shader_rendering->GetProgramID()
		, "BricksCoords"), Model::getInstance()->getNumberTotalTiles(), Model::getInstance()->getBricksPositions());
	  
	gl::Shader::Unbind();

	// VBO, VAO, IBO
	quad_vao = new gl::VAO(1);
	quad_vao->Bind();

	quad_vbo = new gl::BO(GL_ARRAY_BUFFER);
	quad_ibo = new gl::BO(GL_ELEMENT_ARRAY_BUFFER);

	// bind the VBO to the VAO
	quad_vbo->SetBufferData(sizeof(VERTICES), VERTICES, GL_STATIC_DRAW);

	quad_vao->SetVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, sizeof(VERTICES[0]), (GLvoid*)0);

	// bind the IBO to the VAO
	quad_ibo->SetBufferData(sizeof(INDICES), INDICES, GL_STATIC_DRAW);

	gl::VAO::Unbind();
}

void Renderer::render(Camera* cam)
{
	if (vr_outdated)
	{
		updateShaderParams();
		vr_outdated = false;
	}

	ViewMatrix = cam->lookAt();
	shader_rendering->Bind();
	shader_rendering->SetUniform("ViewMatrix", ViewMatrix);
	shader_rendering->BindUniform("ViewMatrix");

	// Render quad on screen
	{
		glCullFace(GL_BACK);
		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
		quad_vao->DrawElements(GL_TRIANGLES, 6, GL_UNSIGNED_INT);
	}

	gl::VAO::Unbind();
	gl::Shader::Unbind();
}

void Renderer::resize(int w, int h)
{
	/* DEBUG */
	m_fovy = 45.0f;
	m_aspect_ratio = (float)w / (float)h;
	ProjectionMatrix = glm::perspective(m_fovy, m_aspect_ratio, 1.0f, 1000.0f);
	m_fovy_tangent = (float)tan(DEGREE_TO_RADIANS(m_fovy) / 2.0);
	m_VerPos = glm::vec3(0.f, 0.f, 0.f);
	/* ********** */

	shader_rendering->Bind();
	shader_rendering->SetUniform("ProjectionMatrix", ProjectionMatrix);
	shader_rendering->BindUniform("ProjectionMatrix");
	shader_rendering->SetUniform("fov_y_tangent", m_fovy_tangent);
	shader_rendering->BindUniform("fov_y_tangent");
	shader_rendering->SetUniform("aspect_ratio", m_aspect_ratio);
	shader_rendering->BindUniform("aspect_ratio");
	shader_rendering->Unbind();

	gl::Shader::Unbind();
}

void Renderer::updateShaderParams()
{
	shader_rendering->Bind();
	updateTreeCut(0);
	glUniform3fv(glGetUniformLocation(shader_rendering->GetProgramID()
		, "BricksCoords"), Model::getInstance()->getNumberTotalTiles(), Model::getInstance()->getBricksPositions());
	gl::Shader::Unbind();
}

/* FUNÇÕES DE DEBUG */
void Renderer::showShaderCalculus()
{
	if (counterParam == 40)
	{
		showParam = true;
		counterParam = 0;
	}

	if (showParam)
	{
		float sX = m_s1;
		glm::vec3 vert_eye = -(glm::vec3(ViewMatrix[3])) * glm::mat3(ViewMatrix);
		glm::vec3 vert_dir = glm::vec3(m_VerPos.x*m_fovy_tangent*m_aspect_ratio, m_VerPos.y*m_fovy_tangent, -1.0) * glm::mat3(ViewMatrix);

		getIntersectRayBox(vert_eye, vert_dir, m_s1, m_tnear, m_tfar);
		glm::vec3 pos = (vert_eye + (vert_dir*m_tnear)) + (glm::vec3(VOS/2, VOS / 2, VOS / 2));
		glm::vec3 difVec = pos - vert_eye;
		float dif = 0;
		if(difVec.x < 0 && difVec.y < 0 && difVec.z < 0)
			dif = std::sqrtf(difVec.x*difVec.x + difVec.y*difVec.y + difVec.z*difVec.z);
		// Printa apenas se câmera mexer 
		if (sX != m_s1)
		{	
			std::cout << "//////////////////////////////////////////////////////" << std::endl;
			printf("vert_eye -- x: %f; y: %f; z: %f \n", vert_eye.x, vert_eye.y, vert_eye.z);
			printf("vert_dir -- x: %f; y: %f; z: %f \n", vert_dir.x, vert_dir.y, vert_dir.z);
			printf("s1 value -- : %f; tnear: %f; tfar: %f \n", m_s1, m_tnear, m_tfar);
			printf("POSITION -- x: %f; y: %f; z: %f \n", pos.x, pos.y, pos.z);
			printf("DIF -- : %f; \n", dif);
			std::cout << "//////////////////////////////////////////////////////" << std::endl;
			std::cout << std::endl;
		}

		showParam = false;
	}
	counterParam++;
}
bool Renderer::getIntersectRayBox(glm::vec3 eye, glm::vec3 dir, float& s1, float& rtnear, float& rtfar)
{
	glm::vec3 aabb_box_min = +m_volume_dim * 0.5f;
	glm::vec3 aabb_box_max = -m_volume_dim * 0.5f;

	// Find intersection with box
	float tnear, tfar;
	bool hit = intersectBox(eye, glm::normalize(dir), aabb_box_min, aabb_box_max, tnear, tfar);

	if (hit)
	{
		if (tnear < 0.0) tnear = 0.0;

		s1 = abs(tfar - tnear) / 1.0;

		rtnear = tnear;
		rtfar = tfar;
	}
	// Apenas para teste
	else
	{
		rtnear = 0.0;
		rtfar = 0.0;
		s1 = 0.0;
	}

	return hit;
}
bool Renderer::intersectBox(glm::vec3 eye, glm::vec3 dir, glm::vec3 min, glm::vec3 max, float& ts, float& tf)
{
	glm::vec3 invR = glm::vec3(1.0,1.0,1.0) / dir;

	glm::vec3 tMin = invR * (min - eye);
	glm::vec3 tMax = invR * (max - eye);
	
	glm::vec3 t1 = glm::min(tMin, tMax);
	glm::vec3 t2 = glm::max(tMin, tMax);

	float tnear = std::max(std::max(t1.x, t1.y), t1.z);
	float tfar = std::min(std::min(t2.x, t2.y), t2.z);

	ts = tnear;
	tf = tfar;

	return tfar > tnear;
}
void genRandomColors()
{
	ofstream myfile;
	myfile.open("colors.txt");
	std::vector<std::vector<std::vector<glm::vec3>>> colors;
	std::vector<std::vector<glm::vec3>> jaux;
	std::vector<glm::vec3> kaux;
	srand(time(0));
	for (int i = 0; i < 4; i++)
	{
		jaux.clear();
		for (int j = 0; j < 4; j++)
		{
			kaux.clear();
			for (int k = 0; k < 4; k++)
			{
				float r1 = ((float)rand() / (RAND_MAX));
				float r2 = ((float)rand() / (RAND_MAX));
				float r3 = ((float)rand() / (RAND_MAX));
				kaux.push_back(glm::vec3(r1, r2, r3));
			}
			jaux.push_back(kaux);
		}
		colors.push_back(jaux);
	}

	for (int i = 0; i < 4; i++)
	{
		for (int j = 0; j < 4; j++)
		{
			for (int k = 0; k < 4; k++)
			{
				myfile << "if(tileIndex.x == " + std::to_string(i) + " && tileIndex.y == " + std::to_string(j) + " && tileIndex.z == " + std::to_string(k) + ")" << std::endl;
				myfile << "{" << std::endl;
				//myfile << "return vec4(1.0/tileIndex.x, 1.0/tileIndex.y, 0.5/tileIndex.z, 1.0);" << std::endl;
				myfile << "return vec4(" + to_string(colors[i][j][k].x) + "," + to_string(colors[i][j][k].y) + "," + to_string(colors[i][j][k].z) + ",1.0);" << std::endl;
				myfile << "}" << std::endl;
			}
		}
	}

	myfile.close();
}