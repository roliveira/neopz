//
// C++ Implementation: pzlog
//
// Description:
//
//
// Author: Philippe R. B. Devloo <phil@fec.unicamp.br>, (C) 2006
//
// Copyright: See COPYING file that comes with this distribution
//
//

#include "pzlog.h"
#include <sys/stat.h>
#include <iostream>

#ifdef LOG4CXX
pthread_mutex_t glogmutex = PTHREAD_MUTEX_INITIALIZER;

#endif

void InitializePZLOG()
{
  std::string path;
  std::string configfile;
#ifdef HAVE_CONFIG_H
  path = PZSOURCEDIR;
  path += "/Util/";
#else
  path = "";
#endif
  configfile = path;
  configfile += "log4cxx.cfg";

#ifndef WIN32
	int res = mkdir ("LOG", S_IRWXU | S_IXGRP | S_IRGRP | S_IXOTH | S_IROTH);
	if (res) std::cout << "Error in mkdir : " << res << std::endl;
#endif

  InitializePZLOG(configfile);
}

