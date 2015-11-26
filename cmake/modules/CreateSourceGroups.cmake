FUNCTION(CreateSourceGroups SOURCE_FILES RELATIVE_TO)
    FOREACH(FILE ${SOURCE_FILES}) 
        FILE(RELATIVE_PATH RELATIVE_SOURCE_PATH ${RELATIVE_TO} ${FILE})
        GET_FILENAME_COMPONENT(PATH_COMPONENT ${RELATIVE_SOURCE_PATH} DIRECTORY)
        GET_FILENAME_COMPONENT(EXTENSION ${RELATIVE_SOURCE_PATH} EXT)
        IF (${EXTENSION} STREQUAL ".h")
            SET(GROUP "${PATH_COMPONENT}\\Header Files")
        ELSE()
            SET(GROUP "${PATH_COMPONENT}\\Source Files")
        ENDIF()
        SOURCE_GROUP(${GROUP} FILES ${FILE})
    ENDFOREACH()
ENDFUNCTION(CreateSourceGroups)