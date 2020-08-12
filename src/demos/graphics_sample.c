/*
 * Working with animation loops.
 */
#include <calendon/cn.h>
#include <calendon/anim-loop.h>
#include <calendon/assets.h>
#include <calendon/log.h>
#include <calendon/math2.h>
#include <calendon/path.h>
#include <calendon/render.h>
#include <calendon/render-resources.h>
#include <calendon/time.h>

#include <math.h>

CnLogHandle LogSysSample;

CnAnimationLoopCursor sampleCursor;
CnAnimationLoop sampleLoop;

CnTransform2 rotate;

#define SPRITE_ANIMATION_FRAMES 3
CnSpriteId spriteFrames[SPRITE_ANIMATION_FRAMES];

#define NUM_CIRCLE_VERTICES 20
CnFloat2 circleOrigin;
CnFloat2 circleVertices[NUM_CIRCLE_VERTICES];

CnFontId font;
static CnTime lastDt;

/**
 * Creates a line of points to form circle in a counter clockwise winding.
 */
void cnRLL_CreateCircle(CnFloat2* vertices, uint32_t numVertices, float radius)
{
	CN_ASSERT(vertices != NULL, "Cannot write vertices into a null pointer");
	CN_ASSERT(radius > 0.0f, "Radius must positive: %f provided", radius);
	const float arcSize = 2 * 3.14159f / (numVertices - 1);
	for (uint32_t i = 0; i < numVertices - 1; ++i) {
		vertices[i] = cnFloat2_Make(radius * cosf(i * arcSize),
									radius * sinf(i * arcSize));
	}
	vertices[numVertices - 1] = vertices[0];
}

static void drawScene(void)
{
	cnR_DrawDebugFullScreenRect();

	CnFloat2 position = cnFloat2_Make(100, 100);
	CnDimension2f size = { .width = 100.0f, .height = 100.0f };
	cnR_DrawSprite(spriteFrames[sampleCursor.current], position, size);

	CnOpaqueColor red = cnOpaqueColor_MakeRGBu8(255, 0, 0);
	CnOpaqueColor green = cnOpaqueColor_MakeRGBu8(0, 255, 0);
	CnOpaqueColor blue = cnOpaqueColor_MakeRGBu8(0, 0, 255);

	cnR_DrawDebugLineStrip(circleVertices, NUM_CIRCLE_VERTICES, red);
	static int step = 0;
	--step;
	if (step < 0) step = NUM_CIRCLE_VERTICES - 2;
	cnR_DrawDebugLine(circleOrigin.x, circleOrigin.y, circleVertices[step].x, circleVertices[step].y, green);

	CnTransform2 smallRotate = cnTransform2_MakeRotation(cnPlanarAngle_MakeDegrees(1));
	rotate = cnTransform2_Combine(rotate, smallRotate);
	const CnTransform2 translate = cnTransform2_MakeTranslateXY(600, 300);
	const CnTransform2 transform = cnTransform2_Combine(translate, rotate);

	const CnDimension2f dimensions = (CnDimension2f) { .width = 200.0f, .height = 300.0f};
	cnR_OutlineRect(cnFloat2_Make(0, 0), dimensions, blue, transform);

	const CnAABB2 aabb = cnAABB2_MakeMinMax(
		cnFloat2_Make(-dimensions.width / 2.0f, -dimensions.height / 2.0f),
		cnFloat2_Make(dimensions.width / 2.0f, dimensions.height / 2.0f));
	CnAABB2 box = cnMath2_TransformAABB2(aabb, rotate);
	cnR_OutlineRect(cnAABB2_Center(box), (CnDimension2f) { box.max.x - box.min.x, box.max.y - box.min.y }, red, translate);

	cnR_DrawSimpleText(font, cnFloat2_Make(000, 500), "Hello, Paul!\xe2\x86\x93→\xe2\x86\x92");
	cnR_DrawSimpleText(font, cnFloat2_Make(000, 600), "«café, caffè» ™ © Â ←");

	static char frameTime[100] = "";
	lastDt = cnTime_Max(cnTime_MakeMilli(1), lastDt);
	static int fpsTick = 0;
	if (++fpsTick % 10 == 0) {
		fpsTick = 0;
		cnString_Format(frameTime, 100, "FPS: %.1f", 1000.0f / cnTime_Milli(lastDt));
	}
	cnR_DrawSimpleText(font, cnFloat2_Make(0, 700), frameTime);
}

CN_GAME_API bool CnPlugin_Init(void)
{
	cnLog_RegisterSystem(&LogSysSample, "Sample", CnLogVerbosityTrace);
	CN_TRACE(LogSysSample, "Sample loaded");

	CN_TRACE(LogSysSample, "Animation loop size:        %zu bytes", sizeof(CnAnimationLoop));
	CN_TRACE(LogSysSample, "Animation loop cursor size: %zu bytes", sizeof(CnAnimationLoopCursor));

	sampleLoop.numStates = SPRITE_ANIMATION_FRAMES;
	sampleLoop.elapsed[0] = cnTime_MakeMilli(150);
	sampleLoop.elapsed[1] = cnTime_MakeMilli(150);
	sampleLoop.elapsed[2] = cnTime_MakeMilli(150);

	cnR_CreateSprite(&spriteFrames[0]);
	cnR_CreateSprite(&spriteFrames[1]);
	cnR_CreateSprite(&spriteFrames[2]);

	const char* frameFilenames[] = {
		"sprites/stick_person.png",
		"sprites/stick_person2.png",
		"sprites/stick_person3.png"
	};

	for (uint32_t i = 0; i < 3; ++i) {
		CnPathBuffer path;
		cnAssets_PathBufferFor(frameFilenames[i], &path);
		cnR_LoadSprite(spriteFrames[i], path.str);
	}

	rotate = cnTransform2_MakeIdentity();

	circleOrigin = cnFloat2_Make(200.0f, 300.0f);
	cnRLL_CreateCircle(circleVertices, NUM_CIRCLE_VERTICES, 50.0f);
	for (uint32_t i = 0; i < NUM_CIRCLE_VERTICES; ++i) {
		circleVertices[i] = cnFloat2_Add(circleVertices[i], circleOrigin);
	}

	CnPathBuffer fontPath;
	cnAssets_PathBufferFor("fonts/bizcat.psf", &fontPath);
	cnR_CreateFont(&font);
	if (!cnR_LoadPSF2Font(font, fontPath.str))	{
		CN_FATAL_ERROR("Unable to load font: %s", fontPath.str);
	}
	return true;
}

CN_GAME_API void CnPlugin_Draw(void)
{
	cnR_StartFrame();

	const CnAABB2 backingArea = cnR_BackingCanvasAABB2();
	const CnFloat2 center = cnAABB2_Center(backingArea);

	const CnAABB2 bottomLeft = cnAABB2_MakeMinMax(cnFloat2_Make(0.0f, 0.0f), center);
	cnR_SetViewport(bottomLeft);
	cnR_SetCameraAABB2(bottomLeft);
	drawScene();

	const CnAABB2 topRight = cnAABB2_MakeMinMax(center, backingArea.max);
	cnR_SetViewport(topRight);
	cnR_SetCameraAABB2(topRight);
	drawScene();

	cnR_SetCameraAABB2(cnR_BackingCanvasAABB2());
	const CnAABB2 topLeft = cnAABB2_MakeMinMax(cnFloat2_Make(0.0f, center.y), cnFloat2_Make(center.x, backingArea.max.y));
	cnR_SetViewport(topLeft);
	drawScene();

	const CnAABB2 bottomRight = cnAABB2_MakeMinMax(cnFloat2_Make(center.x, 0.0f), cnFloat2_Make(backingArea.max.x, center.y));
	cnR_SetViewport(bottomRight);
	drawScene();

	cnR_EndFrame();
}

CN_GAME_API void CnPlugin_Tick(CnTime dt)
{
	cnAnimLoop_Tick(&sampleLoop, &sampleCursor, dt);
	lastDt = dt;
}

CN_GAME_API void CnPlugin_Shutdown(void)
{
}
