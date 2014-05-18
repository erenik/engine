/// Emil Hedemalm
/// 2014-04-16
/// Enums of the categories and IDs of all implemented filter-types.


#ifndef CV_FILTER_TYPES_H
#define CV_FILTER_TYPES_H

// Filter-types, defines general/main output of a filter. Also used for sorting when in menus.
namespace CVFilterType {
	enum {
		IMAGE_FILTER,
		DATA_FILTER,
		RENDER_FILTER,
	};
};

// Enum with all known filter-types. NOTE: Try to ensure that each ID remains the same.
namespace CVFilterID {
	enum {
		/// Image-manipulation filters.
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
		
		/// Data-filters.
		FIND_CONTOURS = 101,
		FIND_CONVEX_HULLS,
		SHI_TOMASI_CORNERS,
		HOUGH_CIRCLES,
		HOUGH_LINES,
		FIND_QUADS,
		MAX_BOX_PERSISTANCE,
		HAND_DETECTOR,
		QUAD_ASPECT_RATIO_CONSTRAINT,
		HAND_PERSISTENCE,
		HAND_DETECTOR_2,
		CALC_CONVEX_HULL,
		CALC_CONVEXITY_DEFECTS,
		FILTER_LINES_BY_ANGLE,
		MERGE_LINES,
		APPROXIMATE_POLYGONS,
		FINGER_ACTION,

		// Render-filters
		VIDEO_WRITER = 201,
		IMAGE_GALLERY_HAND,
		MOVIE_PROJECTOR,
		MUSIC_PLAYER,


		// Own filters <- Old stuff. Place in either Data or image above.
		LINE_GATHER = 301,
		MAX_BOUNDING_BOX,
		TO_CHAR,

		MAX_FILTERS
	};
};

#endif
