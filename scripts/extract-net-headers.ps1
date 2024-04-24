# env: PROJECT_ROOT PLATFORM

$libs = @('align', 'asio', 'assert', 'beast', 'bind', 'config', 'container_hash', 'core', 'date_time', 'describe', 'detail', 'endian', 'exception', 'intrusive', 'io', 'logic', 'move', 'mp11', 'mpl', 'numeric/conversion', 'optional', 'predef', 'preprocessor', 'smart_ptr', 'static_assert', 'static_string', 'system', 'throw_exception', 'type_traits', 'utility', 'winapi')

# $libs = @('align')

foreach ($lib in $libs) {
    Copy-Item -Path "$PROJECT_ROOT/deps/vcpkg/installed/$PLATFORM/include/boost/$lib/" -Destination "$PROJECT_ROOT/deps/boost/include/" -Recurse -Force
}
