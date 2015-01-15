/// Emil Hedemalm
/// 2014-04-16
/// Enums of the categories and IDs of all implemented filter-types.


#ifndef CV_FILTER_TYPES_H
#define CV_FILTER_TYPES_H

// Filter-types, defines general/main output of a filter. Also used for sorting when in menus.
namespace CVFilterType 
{
	enum {
		IMAGE_FILTER,
		DATA_FILTER,
		RENDER_FILTER,
	};
};

// Enum with all known filter-types. NOTE: Try to ensure that each ID remains the same, or save/load will start failing.
namespace CVFilterID {
	enum {
		////////////////////////////////////////////////////////////////////
		/// Image-manipulation filters.
		////////////////////////////////////////////////////////////////////
		GREYSCALE = 1,
		SCALE_UP,
		SCALE_DOWN,
		CANNY_EDGE,
		INVERT,
		HARRIS_CORNER,
		GAUSSIAN_BLUR,
		THRESHOLD,
		ABS,
		COLOR_FILTER,
		SATURATION_FILTER,
		REMOVE_BACKGROUND,
		ERODE,
		DILATE,
		EXTRACT_CHANNELS,
		HUE_FILTER,
		VALUE_FILTER,
		FILL_THREE,
		ADAPTIVE_THRESHOLD,
		PER_PIXEL_ADAPTIVE_THRESHOLD,
		RENDER_REGION_OF_INTEREST,
		REGION_OF_INTEREST_FILTER,
		LOAD_INITIAL_INPUT,
		OPTICAL_FLOW_FILTER, // Based on the output of the optical flow LK (Lucas-Kanade) filter.
		CONTRAST_BRIGHTNESS,
		SHORT_TO_UNSIGNED_BYTE,
		RANDOM_IMAGE,
		BIT_FILTER,
		HORIZONTAL_DEWARP,
		
		////////////////////////////////////////////////////////////////////
		/// Data-filters.
		////////////////////////////////////////////////////////////////////
		FIND_CONTOURS = 101,
		FIND_CONVEX_HULLS,
		SHI_TOMASI_CORNERS,
		HOUGH_CIRCLES,
		HOUGH_LINES,
		FIND_QUADS,
		MAX_BOX_PERSISTANCE,
		HAND_DETECTOR_OLD_OBSOLETE,	// Made obsolete. Was used when testing briefly.
		QUAD_ASPECT_RATIO_CONSTRAINT,
		HAND_PERSISTENCE,
		HAND_DETECTOR,	// Own hand-detector.
		CALC_CONVEX_HULL,
		CALC_CONVEXITY_DEFECTS,
		FILTER_LINES_BY_ANGLE,
		MERGE_LINES,
		APPROXIMATE_POLYGONS,
		FINGER_ACTION,
		CONTOUR_CLASSIFICATION,
		FINGER_EXTRACTION, // New finger extraction via distance-based histogram analysis
		HAND_EMULATOR,
		/** Identifies which finger is which. Also performs filtering if both the convexity defects 
			and contour segmentation approaches were used beforehand.
		*/
		FINGER_IDENTIFICATION, 
		CONTOUR_SEGMENT_CLASSIFIER, // Was a part of the finger extraction filter, now an own filter. Categorizes segments within each contour.
		CONTOUR_SEGMENTER, // Was also part of inger extraction filter, calls on all contours to generate segments based on a certain length.
		// Based on Shi-Tomasi corner detection as input.
		// Whole image optical flow analysis.
		OPTICAL_FLOW_FARNEBACK,
		OPTICAL_FLOW_LUCAS_KANADE,
		OPTICAL_FLOW_LK_SUBDIVIDED, // Custom subdivision mode on Lukas-Kanade optical flow calculation.
		OPTICAL_FLOW_SIMPLE_FLOW, // http://graphics.berkeley.edu/papers/Tao-SAN-2012-05/
		OPTICAL_FLOW_DUAL_TV_L1, 
		GENERATE_OPTICAL_FLOW_FIELD, // LK-based flow field.
		APPROXIMATE_HAND_VEOCITY_BY_OPTICAL_FLOW,
		FILTER_HANDS_BY_MOVEMENT,
		PRINCIPAL_COMPONENT_ANALYSIS,
		POINT_CLOUDS,
		POINT_CLOUD_POSITIONAL_FILTER,
		HANDS_FROM_POINT_CLOUDS,
		LINE_PERSISTENCE,
		TEMPLATE_MATCHER,
		SWIPE_GESTURE,

		////////////////////////////////////////////////////////////////////
		// Render-filters
		////////////////////////////////////////////////////////////////////
		VIDEO_WRITER = 201,
		IMAGE_GALLERY_HAND,
		MOVIE_PROJECTOR,
		MUSIC_PLAYER,
		CONTOUR_PARTICLES,
		FINGER_PARTICLES,
		HAND_PARTICLES,
		MODEL_VIEWER,
		PONG, // Yup, the game.
		BREAKOUT, // game
		SPACE_SHOOTER, 
		MENU_INTERACTION,
		OPTICAL_FLOW_PARTICLES,
		SPRITE_ANIMATION,
		INTERACTIVE_PLATE,
		PIANO,
		INTERACTIVE_SPHERE,

		// Own filters <- Old stuff. Place in either Data or image above.
		LINE_GATHER = 301,
		MAX_BOUNDING_BOX,
		TO_CHAR,

		MAX_FILTERS
	};
};

#endif
