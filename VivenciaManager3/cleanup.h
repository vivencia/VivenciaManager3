#ifndef CLEANUP_H
#define CLEANUP_H

typedef void ( *VMCleanUpFunction ) ();
extern void addPostRoutine ( VMCleanUpFunction func, const bool bEarlyExec = false );
extern void cleanUpApp ();

#endif // CLEANUP_H
