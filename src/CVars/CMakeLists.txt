
IF(WIN32)
	SET(WIN32 1)
ENDIF(WIN32)

INCLUDE_DIRECTORIES( ${CMAKE_SOURCE_DIR} )

# Set version values in CVarVersion.h  (see CVarVersion.h.in)
CONFIGURE_FILE( CVarVersion.h.in CVarVersion.h )


SET( SRCS 
    cvars_tinyxml.cpp  
    cvars_tinyxmlerror.cpp  
    cvars_tinyxmlparser.cpp
    CVar.cpp
    CVarParse.cpp
    Timestamp.cpp
    Trie.cpp
    TrieNode.cpp
   )

SET( HDRS 
    CVarVersion.h
    cvars_tinyxml.h
    CVar.h
    CVarVectorIO.h
    CVarMapIO.h
    Timestamp.h
    Trie.h
    TrieNode.h
  )

ADD_DEFINITIONS( -DTIXML_USE_STL )

ADD_LIBRARY( cvars ${SRCS} ${HDRS} )

INSTALL( TARGETS cvars ARCHIVE DESTINATION ${CMAKE_INSTALL_PREFIX}/lib )
INSTALL( FILES ${HDRS} DESTINATION ${CMAKE_INSTALL_PREFIX}/include/CVars )

##############################################################################
# Needed by the top level CVarsConfig.cmake
GET_TARGET_PROPERTY( CVARS_LIBRARY cvars LOCATION )
SET( CVARS_LIBRARIES "${CVARS_LIBRARY}"  CACHE PATH "CVARS library" FORCE )
SET( CVARS_INCLUDE_DIR ${CMAKE_SOURCE_DIR} CACHE PATH "CVARS inlcude directories" FORCE )

