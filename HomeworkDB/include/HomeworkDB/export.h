// 一顿操作, 是dll针对提供者和使用者的问题, 编 程 模 板 !
// 按leveldb的结构来, 只有include里的才是对外接口, 才需要HOMEWORKDB_EXPORT

#ifndef HOMEWORKDB_INCLUDE_EXPORT_H_
#define HOMEWORKDB_INCLUDE_EXPORT_H_

#if !defined(HOMEWORKDB_EXPORT)

#if defined(HOMEWORKDB_SHARED_LIBRARY)
#if defined(_WIN32)

#if defined(HOMEWORKDB_COMPILE_LIBRARY)
#define HOMEWORKDB_EXPORT __declspec(dllexport)
#else
#define HOMEWORKDB_EXPORT __declspec(dllimport)
#endif  // defined(HOMEWORKDB_COMPILE_LIBRARY)

#else  // defined(_WIN32)
#if defined(HOMEWORKDB_COMPILE_LIBRARY)
#define HOMEWORKDB_EXPORT __attribute__((visibility("default")))
#else
#define HOMEWORKDB_EXPORT
#endif
#endif  // defined(_WIN32)

#else  // defined(HOMEWORKDB_SHARED_LIBRARY)
#define HOMEWORKDB_EXPORT
#endif

#endif  // !defined(HOMEWORKDB_EXPORT)

#endif  // STORAGE_HOMEWORKDB_INCLUDE_EXPORT_H_
