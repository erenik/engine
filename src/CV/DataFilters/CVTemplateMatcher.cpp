/// Emil Hedemalm
/// 2014-08-26
/// Data filter based on cv::matchTemplate http://docs.opencv.org/modules/imgproc/doc/object_detection.html?highlight=matchtemplate#matchtemplate


#include "CVDataFilters.h"
#include "CV/CVPipeline.h"

#include "File/FileUtil.h"

#undef ERROR

CVTemplateMatcher::CVTemplateMatcher()
	: CVDataFilter(CVFilterID::TEMPLATE_MATCHER)
{
	//	CVFilterSetting * templatesDirectory, * method;
	templatesDirectory = new CVFilterSetting("Templates dir", String("templates"));
	/// Method, see cv documentation.
	method = new CVFilterSetting("Method", 0);
	/// format for the pipeline input and loaded images. Converts as necessary.
	imageFormat = new CVFilterSetting("Image format", 0);
	settings.Add(3, templatesDirectory, method, imageFormat);
	// o.o
}

int CVTemplateMatcher::Process(CVPipeline * pipe)
{
	// Clear old results.
	results.Clear();
	
	// Load templates as needed.
	if (templatesDirectory->HasChanged())
	{
		LoadTemplates(templatesDirectory->GetString());
	}
	
	// Skip if no templates.
	if (templates.Size() == 0)
		return -1;

	// Perform template-matching for each template over the image.
	for (int i = 0; i < templates.Size(); ++i)
	{
		cv::Mat result;
		Template & t = templates[i];
		try{
			cv::matchTemplate(pipe->input, t.cvImage, result, method->GetInt());
		}catch(...)
		{
			continue;
		}
		/// Search result matrix for best results.
	}
	returnType = CVReturnType::CV_TEMPLATE_MATCHES;
	return returnType;
};

bool CVTemplateMatcher::LoadTemplates(String fromDir)
{
	List<String> files;
	bool ok = GetFilesInDirectory(fromDir, files);
	if (!ok)
		return false;

	// Clear old templates.
	templates.Clear();
	// Create new templates.
	for (int i = 0; i < files.Size(); ++i)
	{
		Template t;
		String file = files[i];
		String path = fromDir + "/" + file;
		t.cvImage = cv::imread(path.c_str());

		// Check that image is ok.

		t.name = "Name";
		t.source = path;
		templates.Add(t);
	}
	return true;
}


