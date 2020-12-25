#include "stdafx.h"
#include <time.h>
#include <semaphore.h>
#include <sys/types.h>
#include <sys/ipc.h>
#include <sys/sem.h>
#include <sys/mman.h>
#include <errno.h>
#include <Linux/ShareMemory.h>
#include <CTLogEx.h>

#define FM_SM_LOCK_NAME   "/tmp/sm_lock_%s"
#define FM_SM_NAME        "/sm_%s"

#define PAGE_SIZE (64 * 1024)

#define MODULE_NAME _T("ShareMemory")

union semun
{
    int 			val;
    struct semid_ds *buf;
    unsigned short *array;
    struct seminfo *__buf;
};


static void ShareMemoryLock(int LockId)
{
    if (LockId)
    {
        struct sembuf sem_b;
        sem_b.sem_num = 0;
        sem_b.sem_op = -1;//P()
        sem_b.sem_flg = SEM_UNDO;
        if (semop(LockId, &sem_b, 1) < 0)
        {
            L_ERROR(_T("fb_sm_lock semop -1 failed %d\n"), errno);
        }
    }
}

static void ShareMemoryUnlock(int LockId)
{
    if (LockId)
    {
        struct sembuf sem_b;
        sem_b.sem_num = 0;
        sem_b.sem_op = 1;//v()
        sem_b.sem_flg = SEM_UNDO;
        if (semop(LockId, &sem_b, 1) < 0)
        {
            L_ERROR(_T("fb_sm_lock semop -1 failed %d\n"), errno);
        }
    }
}

CShareMemory::CShareMemory(BOOL bLocal, TCHAR* pFileName)
{
    if (bLocal)
    {
        m_bGlobal = FALSE;
    }
    else
    {
        m_bGlobal = TRUE;
    }

    sprintf(m_szLockFileName, FM_SM_LOCK_NAME, pFileName);
    sprintf(m_szShareMemoryName, FM_SM_NAME, pFileName);
 
    m_iLockId = 0;
    m_iShareMemoryFd = 0;

    m_pDataStart = NULL;
    m_pShareMemory = NULL;
    m_pShareMemorySize = NULL;
    m_dwTotalSize = 0;
    m_dwSize = 0;
    m_dwDataSize = 0;
    m_bCreate = FALSE;
}

CShareMemory::~CShareMemory()
{
    Close();
}

BOOL CShareMemory::InitializeShareMemory()
{
    key_t key = ftok(m_szLockFileName, 0);

    if (key < 0)
    {
        L_ERROR(_T("ftok failed %d\n"), errno);
        return FALSE;
    }

    if (m_bCreate)
    {
        m_iLockId = semget(key, 1, 0666 | IPC_CREAT);
    }
    else
    {
        m_iLockId = semget(key, 1, 0666);
    }
    

    if (m_iLockId < 0)
    {
        L_ERROR(_T("semget failed %d\n"), errno);
        return FALSE;
    }

    if (m_bCreate)
    {
        union semun sem_union;
        sem_union.val = 1;
        if (semctl(m_iLockId, 0, SETVAL, sem_union) < 0)
        {
            L_ERROR(_T("semctl SETVAL failed %d\n"), errno);
        }
    }

    L_INFO(_T("key %d id %d\n"), key, m_iLockId);

    m_iShareMemoryFd = shm_open(m_szShareMemoryName, O_CREAT | O_RDWR, 0777);

    if (m_iShareMemoryFd <= 0)
    {
        L_ERROR(_T("shm_open open failed %d\n"), errno);
        return FALSE;
    }

    if (m_bCreate && m_bGlobal)
    {
        char cmd[256] = { 0 };
        sprintf(cmd, "chmod 777 /dev/shm/%s", &m_szShareMemoryName[1]);
        L_INFO(_T("run %s\r\n"), cmd);
        system(cmd);
    }

    if (m_bCreate)
    {
        m_dwSize = ((m_dwTotalSize / PAGE_SIZE) + 1) * PAGE_SIZE;
        ftruncate(m_iShareMemoryFd, m_dwSize);
        m_pShareMemory = (BYTE*)mmap(NULL, m_dwSize, PROT_READ | PROT_WRITE, MAP_SHARED, m_iShareMemoryFd, 0);
        m_pShareMemorySize = (BYTE*)mmap(NULL, 4, PROT_READ | PROT_WRITE, MAP_SHARED, m_iShareMemoryFd, 0);

        ZeroMemory(m_pShareMemory, m_dwTotalSize);
        m_dwDataSize = m_dwTotalSize - sizeof(DWORD);
        memcpy(m_pShareMemory, &m_dwDataSize, sizeof(DWORD));
    }
    else
    {
        m_pShareMemorySize = (BYTE*)mmap(NULL, 4, PROT_READ | PROT_WRITE, MAP_SHARED, m_iShareMemoryFd, 0);
        memcpy(&m_dwDataSize, m_pShareMemorySize, sizeof(DWORD));

        m_dwTotalSize = m_dwDataSize + sizeof(DWORD);
        m_dwSize = ((m_dwTotalSize / PAGE_SIZE) + 1) * PAGE_SIZE;

        m_pShareMemory = (BYTE*)mmap(NULL, m_dwSize, PROT_READ | PROT_WRITE, MAP_SHARED, m_iShareMemoryFd, 0);
    }

    if (m_pShareMemory == NULL)
    {
        L_ERROR(_T("mmap fail %d \r\n"), GetLastError());
        return FALSE;
    }

    m_pDataStart = m_pShareMemory + sizeof(DWORD);

    return TRUE;
}

BOOL CShareMemory::CreateShareMemory(DWORD nSize)
{
    BOOL bRet = FALSE;
    do
    {
        m_dwTotalSize = sizeof(DWORD) + nSize;

        FILE* fp = fopen(m_szLockFileName, "r+");
        if (fp == NULL)
        {
            m_bCreate = TRUE;
            fp = fopen(m_szLockFileName, "w+");
            L_TRACE(_T("create new key file\r\n"));
        }
        else
        {
            L_ERROR(_T("Sharememory has created\r\n"));
            break;
        }

        if (fp)
        {
            L_TRACE(_T("key file open/create ok\r\n"));
            fclose(fp);
        }

        if (!InitializeShareMemory())
        {
            L_ERROR(_T("InitializeShareMemory failed\r\n"));
            break;
        }

        bRet = TRUE;
    } while (FALSE);

    if (bRet == FALSE)
    {
        Close();
    }

    return bRet;
}

void CShareMemory::Close()
{
    if (m_pShareMemory)
    {
        munmap(m_pShareMemory, m_dwSize);
        m_pShareMemory = NULL;
    }

    if (m_pShareMemorySize)
    {
        munmap(m_pShareMemory, 4);
        m_pShareMemorySize = NULL;
    }

    if (m_iShareMemoryFd != -1)
    {
        close(m_iShareMemoryFd);
    }

    if (m_iLockId)
    {
        union semun sem_union;
        memset(&sem_union, 0, sizeof(union semun));
        semctl(m_iLockId, 0, IPC_RMID, sem_union);
    }

    m_pDataStart = NULL;
    m_pShareMemory = NULL;
    m_pShareMemorySize = NULL;
    m_dwTotalSize = 0;
    m_dwDataSize = 0;
}

BOOL CShareMemory::OpenShareMemory(BOOL bReadOnly)
{
    BOOL bRet = FALSE;
    do
    {
        if (!InitializeShareMemory())
        {
            L_ERROR(_T("InitializeShareMemory failed\r\n"));
            break;
        }

        
        bRet = TRUE;
    } while (FALSE);

    if (bRet == FALSE)
    {
        Close();
    }

    return bRet;
}

int CShareMemory::WriteData(BYTE* pBuffer, int nBufferSize)
{
    if (nBufferSize <= m_dwDataSize)
    {
        memcpy(m_pDataStart, pBuffer, nBufferSize);
        return nBufferSize;
    }
    return 0;
}

int CShareMemory::ReadData(BYTE* pBuffer, int nBufferSize)
{
    int nMin = nBufferSize < m_dwDataSize ? nBufferSize : m_dwDataSize;
    memcpy(pBuffer, m_pDataStart, nMin);
    return nMin;
}

int CShareMemory::GetDataSize()
{
    return m_dwDataSize;
}
