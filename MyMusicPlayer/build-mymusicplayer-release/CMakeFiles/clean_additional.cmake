# Additional clean files
cmake_minimum_required(VERSION 3.16)

if("${CONFIG}" STREQUAL "" OR "${CONFIG}" STREQUAL "Release")
  file(REMOVE_RECURSE
  "CMakeFiles/MyMusicPlayer_autogen.dir/AutogenUsed.txt"
  "CMakeFiles/MyMusicPlayer_autogen.dir/ParseCache.txt"
  "MyMusicPlayer_autogen"
  "_deps/creeper-qt-local/CMakeFiles/creeper-qt-widgets_autogen.dir/AutogenUsed.txt"
  "_deps/creeper-qt-local/CMakeFiles/creeper-qt-widgets_autogen.dir/ParseCache.txt"
  "_deps/creeper-qt-local/creeper-qt-widgets_autogen"
  )
endif()
