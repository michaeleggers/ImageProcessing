#ifndef _STATIC_GEOMETRY_H_
#define _STATIC_GEOMETRY_H_

#include <vector>

#include "dependencies/glm/glm.hpp"
#include "dependencies/glm/ext.hpp"

#include "render_common.h"
#include "batch.h"

void InitStaticGeometry();
void DestroyStaticGeometry();
Batch& GetUnitQuadBatch();

#endif
