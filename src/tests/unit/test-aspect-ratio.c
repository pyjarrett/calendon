#include <calendon/test.h>

#include <calendon/cn.h>
#include <calendon/aspect-ratio.h>

bool cnDimension2u32_Equal(CnDimension2u32 left, CnDimension2u32 right)
{
	return left.width == right.width && left.height == right.height;
}

CN_TEST_SUITE_BEGIN("aspect ratio")
	CN_TEST_UNIT("Same fits same") {
		const CnDimension2u32 source = (CnDimension2u32) { .width = 1024, .height = 768};
		const CnDimension2u32 destination = (CnDimension2u32) { .width = 1024, .height = 768 };

		const CnTransform2 transform = cnAspectRatio_fit(source, destination);
		CN_TEST_ASSERT_TRUE(cnFloat2_DistanceSquared(cnTransform2_Scale(transform),
			cnFloat2_Make(1.0f, 1.0f)) < 0.1f);
		CN_TEST_ASSERT_TRUE(cnFloat2_DistanceSquared(cnTransform2_Translation(transform),
			cnFloat2_Make(0.0f, 0.0f)) < 0.1f);
	}

	CN_TEST_UNIT("Upscaling same size") {
		const CnDimension2u32 source = (CnDimension2u32) { .width = 1024, .height = 768 };
		const CnDimension2u32 destination = (CnDimension2u32) { .width = 2048, .height = 1536 };

		const CnTransform2 transform = cnAspectRatio_fit(source, destination);
		CN_TEST_ASSERT_TRUE(cnFloat2_DistanceSquared(cnTransform2_Scale(transform),
			cnFloat2_Make(2.0f, 2.0f)) < 0.1f);
		CN_TEST_ASSERT_TRUE(cnFloat2_DistanceSquared(cnTransform2_Translation(transform),
			cnFloat2_Make(0.0f, 0.0f)) < 0.1f);
	}

	CN_TEST_UNIT("Downscaling same size") {
		const CnDimension2u32 source = (CnDimension2u32) { .width = 2048, .height = 1536 };
		const CnDimension2u32 destination = (CnDimension2u32) { .width = 1024, .height = 768 };

		const CnTransform2 transform = cnAspectRatio_fit(source, destination);
		CN_TEST_ASSERT_TRUE(cnFloat2_DistanceSquared(cnTransform2_Scale(transform),
			cnFloat2_Make(0.5f, 0.5f)) < 0.1f);
		CN_TEST_ASSERT_TRUE(cnFloat2_DistanceSquared(cnTransform2_Translation(transform),
			cnFloat2_Make(0.0f, 0.0f)) < 0.1f);
	}

	CN_TEST_UNIT("Fit upscaling (narrow to wide)") {
		const CnDimension2u32 source = (CnDimension2u32) { .width = 800, .height = 600 };
		const CnDimension2u32 destination = (CnDimension2u32) { .width = 1920, .height = 1080 };

		const CnTransform2 transform = cnAspectRatio_fit(source, destination);
		CN_TEST_ASSERT_TRUE(cnFloat2_DistanceSquared(cnTransform2_Scale(transform),
			cnFloat2_Make(1.8f, 1.8f)) < 0.1f);

		// The transform should scale to 1440x1080, which needs a 240 unit X translation.
		CN_TEST_ASSERT_TRUE(cnFloat2_DistanceSquared(cnTransform2_Translation(transform),
			cnFloat2_Make(240.0f, 0.0f)) < 0.1f);
	}

	CN_TEST_UNIT("Fit downscaling (wide to narrow)") {
		const CnDimension2u32 source = (CnDimension2u32) { .width = 1920, .height = 1080 };
		const CnDimension2u32 destination = (CnDimension2u32) { .width = 800, .height = 600 };

		const CnTransform2 transform = cnAspectRatio_fit(source, destination);
		CN_TEST_ASSERT_TRUE(cnFloat2_DistanceSquared(cnTransform2_Scale(transform),
			cnFloat2_Make(0.4167f, 0.4167f)) < 0.1f);

		// The transform should scale to 800x450, which needs a 75 unit Y translation.
		CN_TEST_ASSERT_TRUE(cnFloat2_DistanceSquared(cnTransform2_Translation(transform),
			cnFloat2_Make(0.0f, 75.0f)) < 0.1f);
	}
CN_TEST_SUITE_END
