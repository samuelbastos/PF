#include "Reader.h"

#include <cstdio>
#include <fstream>
#include <string>

#include "RawLoader.h"

#include "TransferFunction1D.h"

namespace vr
{
	TransferFunction* ReadTransferFunction(std::string file)
	{
		TransferFunction* ret = NULL;

		int found = file.find_last_of('.');
		std::string extension = file.substr(found + 1);

		printf("---------Reading Transfer Function----------\n");
		printf(" - File: %s\n", file.c_str());

		if (extension.compare("tf1d") == 0)
			ret = ReadTransferFunction_tf1d(file);

		printf("--------------------------------------------\n");
		return ret;
	}

	TransferFunction* ReadTransferFunction_tf1d(std::string file)
	{
		std::ifstream myfile(file);
		if (myfile.is_open())
		{
			std::string interpolation;
			std::getline(myfile, interpolation);
			printf(" - Interpolation: %s\n", interpolation.c_str());

			TransferFunction1D* tf = NULL;
			int init;
			myfile >> init;

			if (init == 1)
			{
				int max_density;
				myfile >> max_density;

				tf = new TransferFunction1D(max_density);

				int cpt_rgb_size;
				myfile >> cpt_rgb_size;
				double r, g, b, a;
				int isovalue;
				for (int i = 0; i < cpt_rgb_size; i++)
				{
					myfile >> r >> g >> b >> isovalue;
					tf->AddRGBControlPoint(TransferControlPoint(r, g, b, isovalue));
				}

				int cpt_alpha_size;
				myfile >> cpt_alpha_size;
				for (int i = 0; i < cpt_alpha_size; i++)
				{
					myfile >> a >> isovalue;
					tf->AddAlphaControlPoint(TransferControlPoint(a, isovalue));
				}
			}
			else
			{
				tf = new TransferFunction1D();

				int cpt_rgb_size;
				myfile >> cpt_rgb_size;
				double r, g, b, a;
				int isovalue;
				for (int i = 0; i < cpt_rgb_size; i++)
				{
					myfile >> r >> g >> b >> isovalue;
					tf->AddRGBControlPoint(TransferControlPoint(r, g, b, isovalue));
				}

				int cpt_alpha_size;
				myfile >> cpt_alpha_size;
				for (int i = 0; i < cpt_alpha_size; i++)
				{
					myfile >> a >> isovalue;
					tf->AddAlphaControlPoint(TransferControlPoint(a, isovalue));
				}
			}
			myfile.close();


			if (interpolation.compare("linear") == 0)
				tf->m_interpolation_type = TFInterpolationType::LINEAR;
			else if (interpolation.compare("cubic") == 0)
				tf->m_interpolation_type = TFInterpolationType::CUBIC;

			int foundname = file.find_last_of('\\');
			std::string tfname = file.substr(foundname + 1);
			tf->SetName(file);

			return tf;
		}
		return NULL;
	}

	Reader::Reader() {}

	Reader::~Reader() {}
}