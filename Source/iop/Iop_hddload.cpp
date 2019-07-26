#include "Iop_Hddload.h"
#include "../Log.h"

#define LOG_NAME "iop_hddload"

using namespace Iop;

std::string CHDDLoad::GetId() const
{
    return "HDDLoad";
}

std::string CHDDLoad::GetFunctionName(unsigned int) const
{
    return "unkown";
}

void CHDDLoad::Invoke(CMIPS& context, unsigned int functionId)
{
    switch(functionId)
	{
	default:
		CLog::GetInstance().Warn(LOG_NAME, "(%08X): Unknown function (%d) called.\r\n",
		                         context.m_State.nPC, functionId);
		break;
	}
}