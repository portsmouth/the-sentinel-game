

#include "FileUtils.h"
#include <Carbon/Carbon.h>

namespace FileUtils {
	
	const int kMaxPathLength = 1024;
	
	//	*** STATIC VARIABLES
	 
	// I'm using a pointer to string rather than just a string here to avoid a potential static initialisation order
	// crash.  If GetApplicationBundlePath() is called from a statically initialised object's constructor, a crash
	// may occur.
	
	std::string *sApplicationBundlePath = NULL;
	
	/*
	 **	GetApplicationBundlePath()
	 ** 
	 **	Returns a string of the form "/Path/To/Your/Application.app".  You can append whatever you want to find
	 **	its enclosed folders.
	 **
	 **	If the bundle couldn't be found for any reason, an empty string is returned.
	 */
	std::string GetApplicationBundlePath()
{
		// Only do it once:
		
		if(sApplicationBundlePath == NULL)
		{
			char path[kMaxPathLength];
			path[0] = 0;
			
			CFBundleRef bundle = ::CFBundleGetMainBundle();
			if(bundle)
			{
				CFURLRef bundle_url = ::CFBundleCopyBundleURL(bundle);
				if(bundle_url)
				{
					CFStringRef path_string = ::CFURLCopyFileSystemPath(bundle_url, kCFURLPOSIXPathStyle);
					if(path_string)
					{
						::CFStringGetCString(path_string, path, kMaxPathLength - 1, kCFStringEncodingASCII);
						::CFRelease(path_string);
						
						sApplicationBundlePath = new std::string(path);
					}
					
					CFRelease(bundle_url);
				}
	    	}
		}
		
		return *sApplicationBundlePath;
}

/*
 **	GetApplicationResourcesFolderPath()
 **
 **	Returns a string containing an absolute path to the application's resources folder.  It will either be like
 **	this (if the application bundle could be found):
 **
 **		"/Path/To/Your/Application.app/Contents/Resources"
 **
 **	Or this (if it couldn't):
 **
 **		"Resources"
 **
 **	Who knows what will happen if the latter string is returned....
 */
std::string GetApplicationResourcesFolderPath()
{
	std::string path = GetApplicationBundlePath();
	if(path.empty())
		path = "Resources";
	else
		path += "/Contents/Resources";
	
	return path;
}



}
