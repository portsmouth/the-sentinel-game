/*
 *  FileUtils.h
 *
 *  Created by Jamie Portsmouth on 19/01/2006.
 *  Copyright 2006 __MyCompanyName__. All rights reserved.
 *
 */

#include "Filenames.h"

#include <Carbon/Carbon.h>
#include <string>

namespace FileUtils {
		
	std::string GetApplicationBundlePath();
	std::string GetApplicationResourcesFolderPath();
			
} 
