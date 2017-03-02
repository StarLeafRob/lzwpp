cc_library(
	name = "decompress_lib",
	srcs = [
		"decompress.cpp",
		"impl.cpp"
	],
	hdrs = [
		"decompress.hpp",
		"impl.hpp"
	],
	visibility = ["//main:__pkg__", "//test:__pkg__"],
)

cc_binary(
	name = "decompressor",
	srcs = [
		"main.cpp"
	],
	deps = [
		"decompress_lib"
	]
)