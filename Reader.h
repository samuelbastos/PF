#pragma once
#include "TransferFunction.h"
#include "Volume.h"

#include <iostream>
#include <cstring>
#include <cstdlib>
#include <string>

namespace vr
{
	TransferFunction* ReadTransferFunction(std::string file);
	TransferFunction* ReadTransferFunction_tf1d(std::string file);

	class Reader
	{
	public:
		Reader();
		~Reader();

	private:

	};
}