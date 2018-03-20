#pragma once

#include "Core/Platform.h"

#ifndef RIO_DEBUG
	#define RIO_DEBUG 0
#endif // RIO_DEBUG

#ifndef RIO_DEVELOPMENT
	#define RIO_DEVELOPMENT 0
#endif // RIO_DEVELOPMENT

#define RIO_RELEASE (!RIO_DEBUG && !RIO_DEVELOPMENT)