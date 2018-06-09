#include "TransferFunction1D.h"

#include "Texture1D.h"
#define GLEW_STATIC
#include <GL/glew.h>

#include <fstream>
#include <cstdlib>

namespace vr
{
	TransferFunction1D::TransferFunction1D(int max_value)
		: m_built(false), m_interpolation_type(TFInterpolationType::LINEAR)
	{
		max_density = max_value;

		m_cpt_rgb.clear();
		m_cpt_alpha.clear();
		m_transferfunction = NULL;
	}

	TransferFunction1D::~TransferFunction1D()
	{
		m_cpt_rgb.clear();
		m_cpt_alpha.clear();
		if (m_transferfunction)
			m_transferfunction;
		if (m_gradients)
			m_gradients;
	}

	const char* TransferFunction1D::GetNameClass()
	{
		return "TrasnferFunction1D";
	}

	void TransferFunction1D::AddRGBControlPoint(TransferControlPoint rgb)
	{
		m_cpt_rgb.push_back(rgb);
	}

	void TransferFunction1D::AddAlphaControlPoint(TransferControlPoint alpha)
	{
		m_cpt_alpha.push_back(alpha);
	}

	void TransferFunction1D::ClearControlPoints()
	{
		m_cpt_rgb.clear();
		m_cpt_alpha.clear();
	}

	gl::Texture1D* TransferFunction1D::GenerateTexture_1D_RGBA()
	{
		if (!m_built)
			Build(m_interpolation_type);

		if (m_transferfunction)
		{
			gl::Texture1D* ret = new gl::Texture1D(m_tflenght);
			ret->GenerateTexture(GL_LINEAR, GL_LINEAR, GL_CLAMP_TO_BORDER);
			float* data = new float[m_tflenght * 4];
			for (int i = 0; i < m_tflenght; i++)
			{
				data[(i * 4)] = (float)(m_transferfunction[i].x);
				data[(i * 4) + 1] = (float)(m_transferfunction[i].y);
				data[(i * 4) + 2] = (float)(m_transferfunction[i].z);
				data[(i * 4) + 3] = (float)(m_transferfunction[i].w);
			}
			ret->SetData((void*)data, GL_RGBA32F, GL_RGBA, GL_FLOAT);
			delete[] data;
			return ret;
		}
		return NULL;
	}

	void TransferFunction1D::Build(TFInterpolationType type)
	{
		if (m_transferfunction)
			delete[] m_transferfunction;
		m_transferfunction = new glm::vec4[max_density + 1];

		if (type == TFInterpolationType::LINEAR)
		{
			int numTF = 0;
			for (int i = 0; i < (int)m_cpt_rgb.size() - 1; i++)
			{
				int steps = m_cpt_rgb[i + 1].m_isoValue - m_cpt_rgb[i].m_isoValue;
				for (int j = 0; j < steps; j++)
				{
					float k = (float)j / (float)(steps);
					glm::vec4 diff = m_cpt_rgb[i + 1].m_color - m_cpt_rgb[i].m_color;
					diff = diff * k;
					m_transferfunction[numTF++] = m_cpt_rgb[i].m_color + diff;
				}
			}
			m_tflenght = numTF;

			numTF = 0;
			for (int i = 0; i < (int)m_cpt_alpha.size() - 1; i++)
			{
				int steps = m_cpt_alpha[i + 1].m_isoValue - m_cpt_alpha[i].m_isoValue;
				for (int j = 0; j < steps; j++)
				{
					float k = (float)j / (float)(steps);
					glm::vec4 diff = m_cpt_alpha[i + 1].m_color - m_cpt_alpha[i].m_color;
					diff = diff * k;
					m_transferfunction[numTF++].w = m_cpt_alpha[i].m_color.w + diff.w;
				}
			}
		}
		else if (type == TFInterpolationType::CUBIC)
		{
			std::vector<TransferControlPoint> tempColorKnots = m_cpt_rgb;
			std::vector<TransferControlPoint> tempAlphaKnots = m_cpt_alpha;

			Cubic* colorCubic = Cubic::CalculateCubicSpline((int)m_cpt_rgb.size() - 1, tempColorKnots);
			Cubic* alphaCubic = Cubic::CalculateCubicSpline((int)m_cpt_alpha.size() - 1, tempAlphaKnots);

			int numTF = 0;
			for (int i = 0; i < (int)m_cpt_rgb.size() - 1; i++)
			{
				int steps = m_cpt_rgb[i + 1].m_isoValue - m_cpt_rgb[i].m_isoValue;
				for (int j = 0; j < steps; j++)
				{
					float k = (float)j / (float)(steps);
					m_transferfunction[numTF++] = colorCubic[i].GetPointOnSpline(k);
				}
			}
			m_tflenght = numTF;

			numTF = 0;
			for (int i = 0; i < (int)m_cpt_alpha.size() - 1; i++)
			{
				int steps = m_cpt_alpha[i + 1].m_isoValue - m_cpt_alpha[i].m_isoValue;
				for (int j = 0; j < steps; j++)
				{
					float k = (float)j / (float)(steps);
					m_transferfunction[numTF++].w = alphaCubic[i].GetPointOnSpline(k).w;
				}
			}
		}
		printf("lqc: Transfer Function 1D Built!\n");
		m_built = true;
	}

	glm::vec4 TransferFunction1D::Get(double value, double max_data_value)
	{
		if (!m_built)
			Build(m_interpolation_type);

		if (max_data_value >= 0)
			value = value * (double(max_density) / max_data_value);

		glm::vec4 vf = glm::vec4(0);
		if (!(value >= 0 && value <= max_density))
			return glm::vec4(0);

		if (value == (float)(m_tflenght - 1))
		{
			return m_transferfunction[m_tflenght - 1];
		}
		else
		{
			glm::vec4 v1 = m_transferfunction[(int)value];
			glm::vec4 v2 = m_transferfunction[((int)value) + 1];

			double t = value - (int)value;

			vf = float(1.0 - t)*v1 + float(t)*v2;
		}

		return vf;
	}

	void TransferFunction1D::PrintControlPoints()
	{
		printf("Print Transfer Function: Control Points\n");
		int rgb_pts = (int)m_cpt_rgb.size();
		printf("- Printing the RGB Control Points\n");
		printf("  Format: \"Number: Red Green Blue, Isovalue\"\n");
		for (int i = 0; i < rgb_pts; i++)
		{
			printf("  %d: %.2f %.2f %.2f, %d\n", i + 1, m_cpt_rgb[i].m_color.x, m_cpt_rgb[i].m_color.y, m_cpt_rgb[i].m_color.z, m_cpt_rgb[i].m_isoValue);
		}
		printf("\n");

		int alpha_pts = (int)m_cpt_alpha.size();
		printf("- Printing the Alpha Control Points\n");
		printf("  Format: \"Number: Alpha, Isovalue\"\n");
		for (int i = 0; i < alpha_pts; i++)
		{
			printf("  %d: %.2f, %d\n", i + 1, m_cpt_alpha[i].m_color.w, m_cpt_alpha[i].m_isoValue);
		}
		printf("\n");
	}

	void TransferFunction1D::PrintTransferFunction()
	{
		printf("Print Transfer Function: Control Points\n");
		printf("  Format: \"IsoValue: Red Green Blue, Alpha\"\n");
		for (int i = 0; i < m_tflenght; i++)
		{
			printf("%d: %.2f %.2f %.2f, %.2f\n", i, m_transferfunction[i].x
				, m_transferfunction[i].y, m_transferfunction[i].z, m_transferfunction[i].w);
		}
	}

	bool TransferFunction1D::Save()//char* filename, TFFormatType format)
	{
		/*
		std::string filesaved;
		filesaved.append (RESOURCE_LIBLQC_PATH);
		filesaved.append ("TransferFunctions/");
		filesaved.append (filename);
		if (format == TFFormatType::LQC)
		{
		filesaved.append (".tf1d");
		std::ofstream myfile (filesaved.c_str ());
		if (myfile.is_open ())
		{
		myfile << 0 << "\n";
		myfile << (int)m_cpt_rgb.size () << "\n";
		for (int i = 0; i < (int)m_cpt_rgb.size (); i++)
		{
		myfile << m_cpt_rgb[i].m_color.x << " " <<
		m_cpt_rgb[i].m_color.y << " " <<
		m_cpt_rgb[i].m_color.z << " " <<
		m_cpt_rgb[i].m_isoValue << " " << "\n";
		}
		myfile << (int)m_cpt_alpha.size () << "\n";
		for (int i = 0; i < (int)m_cpt_alpha.size (); i++)
		{
		myfile << m_cpt_alpha[i].m_color.w << " " <<
		m_cpt_alpha[i].m_isoValue << " " << "\n";
		}
		myfile.close ();
		printf ("lqc: Transfer Function 1D Control Points Saved!\n");
		}
		else
		{
		printf ("lqc: Error on opening file at VRTransferFunction::Save().\n");
		}
		}
		*/

		return true;
	}

	bool TransferFunction1D::Load()//std::string filename, TFFormatType format)
	{
		/*
		std::string filesaved;
		filesaved.append (RESOURCE_LIBLQC_PATH);
		filesaved.append ("TransferFunctions/");
		filesaved.append (filename);
		if (format == TFFormatType::LQC)
		{
		filesaved.append (".tf1d");
		std::ifstream myfile (filesaved.c_str ());
		if (myfile.is_open ())
		{
		int init;
		myfile >> init;

		int cpt_rgb_size;
		myfile >> cpt_rgb_size;
		float r, g, b, a;
		int isovalue;
		for (int i = 0; i < cpt_rgb_size; i++)
		{
		myfile >> r >> g >> b >> isovalue;
		m_cpt_rgb.push_back (TransferControlPoint (r, g, b, isovalue));
		}

		int cpt_alpha_size;
		myfile >> cpt_alpha_size;
		for (int i = 0; i < cpt_alpha_size; i++)
		{
		myfile >> a >> isovalue;
		m_cpt_alpha.push_back (TransferControlPoint (a, isovalue));
		}
		myfile.close ();
		printf ("lqc: Transfer Function 1D Control Points Loaded!\n");
		return true;
		}
		else
		printf ("lqc: Error on opening file at VRTransferFunction::AddControlPointsReadFile().\n");
		}
		return false;
		*/

		return true;
	}
}