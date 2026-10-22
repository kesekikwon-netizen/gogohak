#pragma once

namespace SHDK {
	class IObject {
	public:
		virtual void Release() = 0;
	};

} // namespace SHDK


#include "HwpSDKExport.h"
#include "SdkDef.h"

/**
 * \defgroup Apis
 * @brief SDK에서 사용 가능한 Apis
 */
#include "CtrlCode.h"
#include "ParameterSetArray.h"
#include "ParameterSet.h"
#include "Action.h"
#include "Window.h"
#include "Document.h"
#include "HwpObject.h"

#include "Application.h"


