#pragma once
#include "Shader.h"
#include "Camera.h"
#include "VAO.h"
#include "BO.h"
#include "Texture1D.h"
#include "Texture3D.h"
#include "TransferFunction.h"
#include "Volume.h"
#include "DiskReader.h"

class Renderer
{
public:
	Renderer();
	~Renderer();

	void init(int screenwidth, int screenheight);
	void render(Camera* cam);
	
protected:

	gl::Shader* shader_rendering;
	glm::mat4 ProjectionMatrix, ViewMatrix;
	gl::VAO*  quad_vao;
	gl::BO* quad_vbo;
	gl::BO* quad_ibo;

private:
	
	typedef struct Vertex {
		float Position[3];
	} Vertex;


	void loadDataTest(int brickID);
	void loadData(int brickID);
	void threadfunc();
	void createRenderingPass();
	void resize(int w, int h);
	void updateShaderParams();

	void showShaderCalculus();
	bool getIntersectRayBox(glm::vec3 eye, glm::vec3 dir, float& s1, float& rtnear, float& rtfar);
	bool intersectBox(glm::vec3 eye, glm::vec3 dir, glm::vec3 min, glm::vec3 max, float& tnear, float& tfar);

	bool vr_outdated = true;

	DiskReader* m_reader;

	gl::Texture3D* glsl_bricks_buffer;
	gl::Texture1D* glsl_transfer_function;	// Função de transferência (Texture 1D) ;
	glm::vec3 m_volume_dim;					// Tamanho do volume (scale = 0) (DEBUG);
	glm::vec3 m_brick_dim;

	float m_fovy;							// FOV de Y ; 
	float m_fovy_tangent;					// FOV de Y Tangente ;
	float m_aspect_ratio;					// Aspect Ratio ; 
	float vr_stepsize;						// Step size do cast ;

	// DEBUG SHIT
	bool showParam = true;					// Controla o show do debug ;
	int counterParam = 0;					// Apenas controlar número de vezes que é displayed ;
	glm::vec3 m_VerPos;						// Ponto do vértice da tela (0,0,0) = centro ;
	float m_s1, m_tnear, m_tfar;		    // Coisas do frag shader ;
};

