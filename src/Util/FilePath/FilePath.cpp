
#include "../../OS/OS.h"
#include "FilePath.h"
#include "../../Globals.h"

// This should be improved later on, yo o-o
String FilePath::workingDirectory = "/bin";

FilePath::FilePath()
{}
FilePath::FilePath(String path){
	if (IsAbsolutePath(path)){
		relativePath = MakeRelative(path);
		absolutePath = path;
		return;
	}
	relativePath = path;
};

/// Attempts to make the path relative.
String FilePath::MakeRelative(String path){
	path.Replace('\\', '/');
	if (path.Contains(workingDirectory)){
	//	std::cout<<"\nSource contains \""<<workingDirectory<<"\" string. Removing it and all before it~";
	//	std::cout<<"\nFrom: "<<path<<" ";
		List<String> tokens = path.Tokenize("/");
	//	std::cout<<"\nTokens: "<<tokens.Size();
		for (int i = 0; i < tokens.Size(); ++i){
	//		std::cout<<"\nToken "<<i<<": "<<tokens[i];
			if (tokens[i] == "bin"){
				// Rebuild
	//			std::cout<<"\nBeginning rebuild..";
				path = "";
				for (int j = i+1; j < tokens.Size(); ++j){
	//				std::cout<<"\nAdding "<<tokens[j];
					path += tokens[j];
	//				std::cout<<"\nSource: "<<path;
					if (j < tokens.Size()-1){
						path += "/";
	//					std::cout<<"\nAdding folder /";
					}
				}
				break; // n break loop
			}
		}
	//	std::cout<<": "<<path;
		return path;
	}
	return path;
}

bool FilePath::IsAbsolutePath(String path)
{
    #ifdef WINDOWS
        if (path.Contains(":\\")  || path.Contains(":/"))
            return true;
    #endif
    #ifdef LINUX
        if (path.Length())
        {
            const char* temp = path.c_str();
    //        std::cout << "path: " << temp << std::endl;
            if (temp[0] == '/')
                return true;
        }
    #endif
	return false;
}

/// Returns the last parts after the dots, pretty much.
String FilePath::FileEnding(String path)
{
	for (int i = path.Length() - 1; i > 0; --i){
		/// Find a dot?
		if (path.CharAt(i) == '.'){
			/// Return part from dot to the end!
			return path.Part(i+1);
		}
	}
	return String();
}

/// Returns the string component from the last slash and onwards.
String FilePath::GetFileName(String filePath)
{
	if (filePath.Type() == String::WIDE_CHAR)
		filePath.ConvertToChar();
	for (int i = filePath.Length() - 1; i > 0; --i){
		/// Find a dot?
		if (filePath.CharAt(i) == '\\' ||
			filePath.CharAt(i) == '/')
		{
			/// Return part from dot to the end!
			return filePath.Part(i+1);
		}
	}
	return String();
}