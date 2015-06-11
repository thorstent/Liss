add_library(clang_interf cfg_visitor.cpp thread_visitor.cpp statement_visitor.cpp abstraction_consumer.cpp clang_helpers.cpp)

#add_precompiled_header(clang_interf pch.h
#       FORCEINCLUDE)
