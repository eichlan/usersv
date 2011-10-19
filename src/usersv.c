#include <unistd.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <pwd.h>
#include <grp.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#ifndef GROUP
#define GROUP "svusers"
#endif

#ifndef SVDIR
#define SVDIR ".sv"
#endif

typedef struct UserSv
{
	pid_t pidSv;
	uid_t uidOwner;
	struct UserSv *pNext;
	struct UserSv *pPrev;
} UserSv;

UserSv *pProcHead = NULL;

void addProc( pid_t pidNew, uid_t uidNew )
{
	if( pProcHead == NULL )
	{
		pProcHead = malloc(sizeof(UserSv));
		pProcHead->pidSv = pidNew;
		pProcHead->uidOwner = uidNew;
		pProcHead->pNext = NULL;
		pProcHead->pPrev = NULL;
	}
	else
	{
		UserSv *pTmp = pProcHead;
		pProcHead = malloc(sizeof(UserSv));
		pProcHead->pidSv = pidNew;
		pProcHead->uidOwner = uidNew;
		pProcHead->pNext = pTmp;
		pTmp->pPrev = pProcHead;
	}
}

UserSv *getProc( uid_t uidCheck )
{
	UserSv *pCur = pProcHead;

	while( pCur != NULL )
	{
		if( pCur->uidOwner == uidCheck )
			return pCur;
		pCur = pCur->pNext;
	}

	return NULL;
}

void delProc( UserSv *pUsr )
{
	if( pUsr->pPrev == NULL && pUsr->pNext == NULL )
	{
		free( pProcHead );
		pProcHead == NULL;
	}
	else if( pUsr->pPrev == NULL )
	{
		UserSv *pTmp = pUsr->pNext;
		free( pProcHead );
		pProcHead = pTmp;
		pProcHead->pPrev = NULL;
	}
	else if( pUsr->pNext == NULL )
	{
		UserSv *pTmp = pUsr->pPrev;
		free( pTmp->pNext );
		pTmp->pNext = NULL;
	}
	else
	{
		pUsr->pPrev->pNext = pUsr->pNext;
		pUsr->pNext->pPrev = pUsr->pPrev;
		free( pUsr );
	}
}

void scanProcs()
{
	int iStatus;
	UserSv *pCur = pProcHead;

	while( pCur != NULL )
	{
		if( waitpid( pCur->pidSv, &iStatus, WNOHANG ) )
		{
			if( WIFEXITED( iStatus ) )
			{
				printf("Process %d for user %d ended.\n", pCur->pidSv, pCur->uidOwner );
				UserSv *pTmp = pCur->pNext;
				delProc( pCur );
				pCur = pTmp;
				continue;
			}
		}

		pCur = pCur->pNext;
	}
}

pid_t beginRunsv( char *sUsername, gid_t idGroup )
{
	char *buf;
	struct passwd *usr = getpwnam( sUsername );

	if( usr == NULL )
	{
		printf("Cannot load user record for user: %s\n", sUsername );
		return -1;
	}
	
	if( getProc( usr->pw_uid ) != NULL )
	{
		printf("Already running a service for: %s\n", sUsername );
		return -1;
	}

	if( usr->pw_dir == NULL )
	{
		printf("No homedir is set for user: %s\n", sUsername );
		return -1;
	}

	buf = malloc( strlen( usr->pw_dir ) + 6 );
	strcpy( buf, usr->pw_dir );
	strcat( buf, "/" SVDIR );
	if( access( buf, F_OK ) != 0 )
	{
		printf("Cannot find directory: %s\n", buf );
		free( buf );
		return -1;
	}
	
	pid_t p = fork();
	if( p == 0 )
	{
		initgroups( sUsername, usr->pw_gid );
		// Set user.
		setuid( usr->pw_uid );
		// Exec runsv
		execlp("runsvdir", "runsvdir", buf, "log: ...........................................................................................................................................................................................................................................................................................................................................................................................................", NULL );
		perror("Error executing runsv: ");
		exit( 2 );
	}
	else if( p == -1 )
	{
		// Error
		perror("Error forking: ");
		return -1;
	}

	addProc( p, usr->pw_uid );

	return p;
}

int scanUsers()
{
	struct group *grp;
	char **sName;

	grp = getgrnam( GROUP );
	if( grp == NULL )
	{
		printf("No group name " GROUP " exists.\n");
		return 1;
	}
	printf("Scanning " GROUP " group...\n");
	for( sName = grp->gr_mem; *sName; sName++ )
	{
		beginRunsv( *sName, grp->gr_gid );
	}
}

int main( int argc, char *argv[] )
{
	scanUsers();

	for(;;)
	{
		sleep( 30 );
		scanProcs();
		scanUsers();
	}
}

