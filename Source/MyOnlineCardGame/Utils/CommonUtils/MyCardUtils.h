// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"

//#include "Core.h"
#include "CoreUObject.h"

#include "UObject/Object.h"
#include "UObject/NoExportTypes.h"
#include "MyCardUtils.generated.h"

DECLARE_LOG_CATEGORY_EXTERN(LogMyUtilsI, Display, All);

#define LogMyUtilsInstance LogMyUtilsI

#if _WIN32 || _WIN64 
#define MY_PATH_SPLIT_CHAR TEXT("\\")
#else
#define MY_PATH_SPLIT_CHAR TEXT("/")
#endif

#define MY_GET_ARRAY_LEN(A) ( sizeof(A) / sizeof(A[0]))

inline FString myGetFileNameFromFullPath(FString inPath)
{
    int32 idx = inPath.Find(MY_PATH_SPLIT_CHAR, ESearchCase::CaseSensitive, ESearchDir::FromEnd, -1);
    if (idx >= 0 && (inPath.Len() - idx - 1) > 0) {
        return inPath.Right(inPath.Len() - idx - 1);
    }
    else {
        return inPath;
    }
};

#define UE_MY_LOG(CategoryName, Verbosity, Format, ...) \
        UE_LOG(CategoryName, Verbosity, TEXT("%s:%d: ") Format, *myGetFileNameFromFullPath(TEXT(__FILE__)), __LINE__, ##__VA_ARGS__)

//Fatal cause log not written to disk but core dump, so we don't use Fatal anywhere
#define MY_VERIFY(cond) \
       if (!(cond)) { UE_MY_LOG(LogMyUtilsI, Error, _TEXT("my verify false£º (" #cond ")")); UE_MY_LOG(LogMyUtilsI, Fatal, _TEXT("core dump now")); verify(false); }

       //     if (!(cond)) {UE_MY_LOG(LogMyUtilsI, Error, _TEXT("my verify false")); UE_MY_LOG(LogMyUtilsI, Fatal, _TEXT("core dump now")); verify(false);}


USTRUCT()
struct FMyIdValuePair
{
    GENERATED_USTRUCT_BODY()

    FMyIdValuePair()
    {
        reset(true);
    };

    void reset(bool bResetValue)
    {
        m_iId = -1;
        if (bResetValue) {
            m_iValue = 0;
        }
    };

    inline FString genDebugStr() const
    {

        return FString::Printf(TEXT("(%d, %d)"), m_iId, m_iValue);

    };

    //return false if error happens, means failed
    static bool helperTryRevealValue(int32 &destValue, int32 value)
    {
        if (value > 0) {
            if (destValue > 0) {
                if (destValue != value) {
                    UE_MY_LOG(LogMyUtilsInstance, Error, TEXT("destValue != value:  %d,%d."), destValue, value);
                    return false;
                }
            }
            else {
                destValue = value;
            }
        }
        else {
            UE_MY_LOG(LogMyUtilsInstance, Error, TEXT("Invalid value %d."), value);
            return false;
        }

        return true;
    }

    inline void revealValue(int32 value)
    {
        bool bOK = helperTryRevealValue(m_iValue, value);
        if (!bOK) {
            UE_MY_LOG(LogMyUtilsInstance, Error, TEXT("Trying to resolve a card value but failed: m_iId %d, m_iValue %d, value %d."), m_iId, m_iValue, value);
            MY_VERIFY(false);
        }
    };

    static void helperIdValuePairs2IdsValues(const TArray<FMyIdValuePair> &aIdValuePairs, TArray<int32> *pIds, TArray<int32> *pValues)
    {
        if (pIds) {
            pIds->Reset();
        }
        if (pValues) {
            pValues->Reset();
        }

        int32 l = aIdValuePairs.Num();
        for (int32 i = 0; i < l; i++) {
            if (pIds) {
                pIds->Emplace(aIdValuePairs[i].m_iId);
            }
            if (pValues) {
                pValues->Emplace(aIdValuePairs[i].m_iValue);
            }
        }
    };

    //value will be default (not set)
    static void helperIds2IdValuePairs(const TArray<int32> &aIds, TArray<FMyIdValuePair> &aIdValuePairs)
    {
        aIdValuePairs.Reset();
        int32 l = aIds.Num();
        for (int32 i = 0; i < l; i++) {
            int32 idx = aIdValuePairs.Emplace();
            aIdValuePairs[idx].m_iId = aIds[i];
        }
    };

    UPROPERTY()
    int32 m_iId; // >= 0 means valid

    UPROPERTY()
    int32 m_iValue; // > 0 means valid
};

/*
template<>
struct TIsTriviallyCopyConstructible<FMyIdValuePair>
{
    enum
    {
        value = false
    };
};
*/

USTRUCT(BlueprintType)
struct FMyIdCollectionCpp
{
    GENERATED_USTRUCT_BODY()

    FMyIdCollectionCpp()
    {};


    UPROPERTY(BlueprintReadWrite, meta = (DisplayName = "IDs"))
    TArray<int32> m_aIds;

};

USTRUCT()
struct FMyValueIdMapCpp
{
    GENERATED_USTRUCT_BODY()

    FMyValueIdMapCpp()
    {
        clear();
    };


    FMyValueIdMapCpp& operator = (const FMyValueIdMapCpp& rhs)
    {
        if (this == &rhs) {
            return *this;
        }

        copyDeep(&rhs);
        return *this;
    };

    void clear();

    /**
    * @return false if duplicated one found
    */
    bool insert(int32 id, int32 value);
    bool insert(const FMyIdValuePair &pair);

    /*
    * @return false if not found
    */
    bool remove(int32 id, int32 value);

    /**
    * @return false if duplicated one found
    */
    bool insertIdValuePairs(const TArray<FMyIdValuePair>& aPairs, bool bAssertValueValid = false);
    void insertIdValuePairsAssertNoDup(const TArray<FMyIdValuePair>& aPairs);


    void removeAllByValue(int32 value, TArray<FMyIdValuePair>& outPairs);

    void copyDeep(const FMyValueIdMapCpp *other);

    /*
    * collect IDs by value, order is not guareenteed
    * @param numReq	num to collect, if <= 0, collect every thing
    */
    void collectByValue(int32 value, int32 numReq, TArray<int32>& outIds) const;

    void collectByValueWithValue(int32 value, int32 numReq, TArray<FMyIdValuePair>& outPairs) const;

    void collectAll(TArray<int32>& outIds) const;

    void collectAllWithValue(TArray<FMyIdValuePair> outPairs) const;

    int32 getCountByValue(int32 value) const;

    bool contains(int32 id, int32 value) const;

    /*
    * try find one elem, return false if not found
    */
    bool getOneIdValue(int32 &outId, int32 &outValue) const;

    /**
    * return the elem count
    * @return the count in map
    */
    int32 getCount() const;

    /*
    * return the keys of map, correspond the the "value" part of insert API
    * @param outKeys the keys
    */
    void keys(TArray<int32>& outKeys) const;

    /*
    inline
    const TMap<int32, FMyIdCollectionCpp>& getValueMapRef() const
    {
        return m_mValueMap;
    };
    */

protected:

    //unreal TMAp owns its member
    UPROPERTY()
    TMap<int32, FMyIdCollectionCpp> m_mValueMap;

    UPROPERTY()
    int32 m_iCount;
};

class FMyThreadControlCpp : public FRunnable
{
public:
    FMyThreadControlCpp(uint32 uLoopTimeMs) : FRunnable()
    {
        m_iThreadCount = -1;
        MY_VERIFY(uLoopTimeMs > 0);
        m_uLoopTimeMs = uLoopTimeMs;
        m_bKill = false;
        m_bPause = false;
        m_bStartState = false;

        //Initialize FEvent (as a cross platform (Confirmed Mac/Windows))
        m_pSemaphore = FGenericPlatformProcess::GetSynchEventFromPool(false);
        m_pThread = NULL;

        const uint32 CurrentThreadId = FPlatformTLS::GetCurrentThreadId();
        //UE_MY_LOG(LogMyUtilsInstance, Warning, TEXT("FMyThreadControlCpp() thread id: %d"), CurrentThreadId);
        UE_MY_LOG(LogMyUtilsInstance, Warning, TEXT("FMyThreadControlCpp() thread static count: %d"), FMyThreadControlCpp::s_iThreadCount.GetValue());
    };

    virtual ~FMyThreadControlCpp()
    {
        UE_MY_LOG(LogMyUtilsInstance, Warning, TEXT("~FMyThreadControlCpp(), thread count %d."), m_iThreadCount);
        cleanUp();

    };


    //Use this method to kill the thread!!
    void EnsureCompletion()
    {
        stopNotVirtual();

        if (m_pThread)
        {
            m_pThread->WaitForCompletion();
        }
    };

    //Pause the thread 
    void PauseThread()
    {
        m_bPause = true;
    };

    //Continue/UnPause the thread, same as kick if thread sleeped
    void ContinueThread()
    {
        m_bPause = false;
    };

    bool IsThreadPaused()
    {
        return (bool)m_bPause;
    };

    virtual uint32 Run() override
    {
        //Initial wait before starting
        FPlatformProcess::Sleep(0.03);

        m_bStartState = true;

        beginInRun();

        while (!m_bKill)
        {
            if (m_bPause)
            {
                //FEvent->Wait(); will "sleep" the thread until it will get a signal "Trigger()"
                m_pSemaphore->Wait();

                if (m_bKill)
                {
                    return 0;
                }
            }
            else
            {
                loopInRun();
                m_pSemaphore->Wait(m_uLoopTimeMs, false);
                //FPlatformProcess::Sleep(1);
            }
        }

        endInRUn();

        return 0;
    }

    virtual void Stop() override
    {
        const uint32 CurrentThreadId = FPlatformTLS::GetCurrentThreadId();
        UE_MY_LOG(LogMyUtilsInstance, Warning, TEXT("Stop(), thread id: %d"), CurrentThreadId);

        stopNotVirtual();
    };

    //if false, thread not started, or will end soon
    inline bool isInStartState()
    {
        return m_bStartState;
    };

    void kick()
    {
        m_pSemaphore->Trigger();
    };


    virtual void beginInRun() = NULL;
    virtual void loopInRun() = NULL;
    virtual void endInRUn() = NULL;

    virtual bool Init() override
    {
        const uint32 CurrentThreadId = FPlatformTLS::GetCurrentThreadId();
        UE_MY_LOG(LogMyUtilsInstance, Warning, TEXT("Init() thread id: %d"), CurrentThreadId);
        return true;
    };

    virtual void Exit() override
    {
        const uint32 CurrentThreadId = FPlatformTLS::GetCurrentThreadId();
        UE_MY_LOG(LogMyUtilsInstance, Warning, TEXT("Exit() thread id: %d"), CurrentThreadId);
        m_bStartState = false;
    };

protected:

    void stopNotVirtual()
    {
        m_bKill = true; //Thread kill condition "while (!m_Kill){...}"
        m_bPause = false;

        if (m_pSemaphore)
        {
            //We shall signal "Trigger" the FEvent (in case the Thread is sleeping it shall wake up!!)
            m_pSemaphore->Trigger();
        }
    }

    void cleanUp()
    {
        if (m_pThread)
        {
            EnsureCompletion();

            //Cleanup the worker thread
            delete m_pThread;
            m_pThread = nullptr;
        }

        if (m_pSemaphore)
        {
            //Cleanup the FEvent
            FGenericPlatformProcess::ReturnSynchEventToPool(m_pSemaphore);
            m_pSemaphore = nullptr;
        }
    };


    bool start()
    {
        if (m_pThread == NULL) {
            m_iThreadCount = FMyThreadControlCpp::s_iThreadCount.GetValue();
            FString name = FString::Printf(TEXT("MyMJGAMECoreSubThread_%d"), m_iThreadCount);
            m_pThread = FRunnableThread::Create(this, *name, 0, TPri_Normal);
            s_iThreadCount.Increment();
            return true;
        }
        else {
            return false;
        }
    }

    static FThreadSafeCounter s_iThreadCount;
    int32 m_iThreadCount;
    
    //As the name states those members are Thread safe
    uint32 m_uLoopTimeMs;

    FThreadSafeBool m_bKill;
    FThreadSafeBool m_bPause;
    FThreadSafeBool m_bStartState;

    //FCriticalSection m_mutex;

    FEvent* m_pSemaphore;
    //Thread to run the worker FRunnable on
    FRunnableThread* m_pThread;

};