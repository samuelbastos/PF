#pragma once
#include "TransferFunction.h"

#include <vector>
#include <iostream>

#include <glm/glm.hpp>

namespace vr
{
	class TransferFunction1D : public TransferFunction
	{
	public:
		TransferFunction1D(int max_value = 255.0);
		~TransferFunction1D();

		virtual const char* GetNameClass();
		virtual glm::vec4 Get(double value, double max_data_value = -1.0);

		virtual gl::Texture1D* GenerateTexture_1D_RGBA();

		void AddRGBControlPoint(TransferControlPoint rgb);
		void AddAlphaControlPoint(TransferControlPoint alpha);
		void ClearControlPoints();

		glm::vec3 GetRGBPointOnSpline(float s);
		glm::vec3 GetAlphaPointOnSpline(float s);

		//If we don't have a file with the values of the TF, we need to compute the TF
		void Build(TFInterpolationType type);

		void PrintControlPoints();
		void PrintTransferFunction();

		bool Save();
		bool Load();

		TFInterpolationType m_interpolation_type;
		bool m_built;
	private:
		std::vector<TransferControlPoint> m_cpt_rgb;
		std::vector<TransferControlPoint> m_cpt_alpha;
		glm::vec4* m_transferfunction;
		int m_tflenght;
		glm::vec3* m_gradients;

		int max_density;
	};

}

