// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"

//#include "Core.h"
#include "CoreUObject.h"

#include "UObject/Object.h"
#include "UObject/NoExportTypes.h"
#include "MyCardUtils.generated.h"

#define MyIntIdDefaultInvalidValue (-1)
#define MyUIntIdDefaultInvalidValue (0)

#define MyUInt32IdWarnBottomValue (0xF0000000)
#define MyUInt32IdCriticalBottomValue (0xF8000000)
#define MyUInt32IdCoreDumpBottomValue (0xFFFFFFFF - 10)

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
       if (!(cond)) { UE_MY_LOG(LogMyUtilsI, Error, _TEXT("my verify false: (" #cond ")")); UE_MY_LOG(LogMyUtilsI, Fatal, _TEXT("core dump now")); verify(false); }

       //     if (!(cond)) {UE_MY_LOG(LogMyUtilsI, Error, _TEXT("my verify false")); UE_MY_LOG(LogMyUtilsI, Fatal, _TEXT("core dump now")); verify(false);}


USTRUCT(BlueprintType)
struct FMyIdValuePair
{
    GENERATED_USTRUCT_BODY()

    FMyIdValuePair()
    {
        reset(true);
    };

    void reset(bool bResetValue)
    {
        m_iId = MyIntIdDefaultInvalidValue;
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

USTRUCT(BlueprintInternalUseOnly)
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
        changeKeepOrder(false, false);
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
    const TArray<int32>& getIdsAllCached();

    void collectAllWithValue(TArray<FMyIdValuePair> &outPairs) const;

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
    inline int32 getCount() const
    {
        return m_iCount;
    }

    /*
    * return the keys of map, correspond the the "value" part of insert API
    * @param outKeys the keys
    */
    void keys(TArray<int32>& outKeys) const;

    void changeKeepOrder(bool bKeepOrder, bool bOrderBig2Little)
    {
        if (!bKeepOrder) {
            m_iKeepOrder = 0;
            //invalidCaches();
            return;
        }

        if (bOrderBig2Little) {
            m_iKeepOrder = 2;
        }
        else {
            m_iKeepOrder = 1;
        }

        sortByValue(bOrderBig2Little);
        invalidCaches();
    };

    /*
    inline
    const TMap<int32, FMyIdCollectionCpp>& getValueMapRef() const
    {
        return m_mValueMap;
    };
    */

protected:

    void sortByValue(bool bBig2Little);
    void invalidCaches()
    {
        m_aIdsAllCached.Reset();
    };

    //unreal TMAp owns its member
    UPROPERTY()
    TMap<int32, FMyIdCollectionCpp> m_mValueMap;

    UPROPERTY()
    int32 m_iCount;

    //0 not keep, 1 little to big, 2 big to little
    UPROPERTY()
    int32 m_iKeepOrder;

    UPROPERTY()
    TArray<int32> m_aIdsAllCached;
};

class FMyRunnableBaseCpp : public FRunnable
{
public:
    FMyRunnableBaseCpp(uint32 uLoopTimeMs = 1000) : FRunnable()
    {
        anyThreadSetLoopTimeMs(uLoopTimeMs);

        m_bKill = false;
        m_bPause = false;
        m_bBeingUsedBySubThread = false;
        m_bInSubThreadLoop = false;

        //Initialize FEvent (as a cross platform (Confirmed Mac/Windows))
        m_pSemaphore = FGenericPlatformProcess::GetSynchEventFromPool(false);

        //const uint32 CurrentThreadId = FPlatformTLS::GetCurrentThreadId();
        //UE_MY_LOG(LogMyUtilsInstance, Warning, TEXT("FMyThreadControlCpp() thread id: %d"), CurrentThreadId);
        //UE_MY_LOG(LogMyUtilsInstance, Warning, TEXT("FMyThreadControlCpp() thread static count: %d"), FMyThreadControlCpp::s_iThreadCount.GetValue());
    };

    virtual ~FMyRunnableBaseCpp()
    {
        //UE_MY_LOG(LogMyUtilsInstance, Warning, TEXT("~FMyThreadControlCpp(), thread count %d."), m_iThreadCount);

        if (!isReadyForMainThreadClean()) {
            UE_MY_LOG(LogMyUtilsInstance, Error, TEXT("Runnable data is still used when destroy, check you code!"));
            MY_VERIFY(false);
        }

        if (m_pSemaphore)
        {
            FGenericPlatformProcess::ReturnSynchEventToPool(m_pSemaphore);
            m_pSemaphore = nullptr;
        }
    };

    virtual FString anyThreadGenName() const = NULL;
    virtual bool subThreadIsReady() const = NULL;

    inline void anyThreadSetLoopTimeMs(uint32 uLoopTimeMs)
    {
        //this function is actually thread safe
        MY_VERIFY(uLoopTimeMs > 0);
        m_uLoopTimeMs = uLoopTimeMs;
    }

protected:

    template <typename T> friend class FMyThreadControlCpp;

    //called in subthread, start

    virtual void subThreadLoopInRun() = NULL;
    virtual bool subThreadInitBeforRun() { return true; };
    virtual void subThreadExitAfterRun() {};

    virtual bool Init() override final
    {
        const uint32 CurrentThreadId = FPlatformTLS::GetCurrentThreadId();

        bool ret;
        if (subThreadInitBeforRun()) {
            m_bInSubThreadLoop = true;
            ret = true;
        }
        else {
            m_bInSubThreadLoop = false;
            ret = false;
        }

        UE_MY_LOG(LogMyUtilsInstance, Display, TEXT("subthread Init() thread id: %d, ret %d."), CurrentThreadId, ret);

        return ret;
    };

    virtual void Exit() override final
    {
        const uint32 CurrentThreadId = FPlatformTLS::GetCurrentThreadId();
        //UE_MY_LOG(LogMyUtilsInstance, Display, TEXT("Exit() thread id: %d"), CurrentThreadId);

        subThreadExitAfterRun();

        m_bInSubThreadLoop = false;
    };

    virtual uint32 Run() override
    {
        const uint32 CurrentThreadId = FPlatformTLS::GetCurrentThreadId();
        UE_MY_LOG(LogMyUtilsInstance, Display, TEXT("sub thread entered Run(), id: %d, m_uLoopTimeMs %d."), CurrentThreadId, m_uLoopTimeMs);

        //Initial wait before starting
        FPlatformProcess::Sleep(0.03);

        while (!m_bKill)
        {
            if (m_bPause)
            {
                //FEvent->Wait(); will "sleep" the thread until it will get a signal "Trigger()"
                m_pSemaphore->Wait();
            }
            else
            {
                subThreadLoopInRun();
                m_pSemaphore->Wait(m_uLoopTimeMs, false);
                //FPlatformProcess::Sleep(1);
            }
        }

        return 0;
    }

    //end

    
    //called in main thread, start:

    virtual void Stop() override
    {
        m_bKill = true; //Thread kill condition "while (!m_Kill){...}"
        setPause(false);
        kick();
    };

    inline
    void setPause(bool bPause)
    {
        bool changed = m_bPause != bPause;
        m_bPause = bPause;

        if (changed) {
            kick();
        }
    };

    inline
    bool getPause() const
    {
        return (bool)m_bPause;
    };

    inline
    void kick()
    {
        m_pSemaphore->Trigger();
    };

    inline bool isReadyForMainThreadClean() const
    {
        return !m_bBeingUsedBySubThread;
    };
    //end

    //note it only increase never, since UE4 have stat mechnism which require different name, and the name we generate with it, so we keep it always increasing
    static FThreadSafeCounter s_iThreadCount;
    
    //As the name states those members are Thread safe
    uint32 m_uLoopTimeMs;

    FThreadSafeBool m_bBeingUsedBySubThread; //main thread to set it
    FThreadSafeBool m_bInSubThreadLoop;
    FThreadSafeBool m_bKill;
    FThreadSafeBool m_bPause;


    //FCriticalSection m_mutex;

    FEvent* m_pSemaphore;
};

//Controller's function can be only called in one thread.
template<typename RunnableType>
class FMyThreadControlCpp
{

public:
    FMyThreadControlCpp()
    {
        m_pThread = NULL;
    };

    virtual ~FMyThreadControlCpp()
    {
        destroy();
    };

    inline
    bool isCreated() const
    {
        return m_pThread != NULL;
    };

    inline RunnableType& getRunnableRef()
    {
        return m_cRunnable;
    };

    //return true if subthread exist, either new created or exist already
    bool create(EThreadPriority ePri = EThreadPriority::TPri_Normal)
    {
        if (!isCreated()) {
            //const uint32 CurrentThreadId = FPlatformTLS::GetCurrentThreadId();

            if (!m_cRunnable.subThreadIsReady()) {
                UE_MY_LOG(LogMyUtilsInstance, Error, TEXT("not ready for usage, runnable name %s."), *m_cRunnable.anyThreadGenName());
                return false;
            }

            FString name = FString::Printf(TEXT("MyControlledThread_%d_%s"), FMyRunnableBaseCpp::s_iThreadCount.GetValue(), *m_cRunnable.anyThreadGenName());

            m_cRunnable.m_bBeingUsedBySubThread = true;
            m_cRunnable.m_bInSubThreadLoop = true;
            m_pThread = FRunnableThread::Create(&m_cRunnable, *name, 0, ePri);

            UE_MY_LOG(LogMyUtilsInstance, Display, TEXT("thread created, thread id: %d, %s."), m_pThread->GetThreadID(), *m_pThread->GetThreadName());

            FMyRunnableBaseCpp::s_iThreadCount.Increment();

            return true;
        }
       
        return true;
    };

    //will halt until subthread detroyed
    void destroy()
    {
        if (isCreated()) {
           // const uint32 CurrentThreadId = FPlatformTLS::GetCurrentThreadId();
            UE_MY_LOG(LogMyUtilsInstance, Display, TEXT("thread destroying, thread id: %d, %s."), m_pThread->GetThreadID(), *m_pThread->GetThreadName());

            m_pThread->Kill(true);
            delete m_pThread;
            m_pThread = nullptr;

            m_cRunnable.m_bBeingUsedBySubThread = false;
            m_cRunnable.m_bInSubThreadLoop = false;

            //FMyRunnableBaseCpp::s_iThreadCount.Decrement();
        }
    };

    inline void kick()
    {
        m_cRunnable.kick();
    };

    //will return quickly
    inline void requestDestroy()
    {
        m_cRunnable.Stop();
    };

    inline bool canBeDestroyedQuickly()
    {
        return !m_cRunnable.m_bInSubThreadLoop;
    };

protected:

    RunnableType m_cRunnable; //owned by main thread
    FRunnableThread* m_pThread;
};

//Warning:: don't use Uobject * here!
template<typename ItemType, EQueueMode Mode = EQueueMode::Spsc>
struct FMyQueueWithLimitBuffer
{
    FMyQueueWithLimitBuffer(FString sDebugName, uint32 uiBufferCount = 32)
    {
        m_uiBufferCount = 0;
        m_sDebugName = sDebugName;
        m_bDebugLackBufferForProduce = false;
        addBuffer(uiBufferCount);
    };

    virtual ~FMyQueueWithLimitBuffer()
    {
        ItemType *pRet;
        pRet = NULL;
        while (m_cQueueProduced.Dequeue(pRet)) {
            delete(pRet);
        };
        pRet = NULL;
        while (m_cQueueConsumed.Dequeue(pRet)) {
            delete(pRet);
        };
    };

    inline void addBuffer(uint32 uiBufferCount)
    {
        ItemType* pNew = NULL;
        for (uint32 i = 0; i < uiBufferCount; i++) {
            pNew = new ItemType();
            m_cQueueConsumed.Enqueue(pNew);
        }
        m_uiBufferCount += uiBufferCount;

        checkType2(pNew);
    };

    //void checkType();

    void checkType2(UObject* p)
    {
        UE_MY_LOG(LogMyUtilsInstance, Error, TEXT("we don't support UObject* as itemType yet!"));
        //MY_VERIFY(false);
    };

    void checkType2(...)
    {

    };

    inline
    ItemType* getItemForProduce()
    {
        ItemType *pItem = NULL;
        m_cQueueConsumed.Dequeue(pItem);

        if (m_bDebugLackBufferForProduce != (pItem == NULL)) {
            UE_MY_LOG(LogMyUtilsInstance, Warning, TEXT("buffer (%s) for produce lacking state change %d->%d!"), *m_sDebugName, m_bDebugLackBufferForProduce, (pItem == NULL));
        }
        m_bDebugLackBufferForProduce = (pItem == NULL);

        return pItem;
    };

    inline
    void putInProducedItem(ItemType* &pItem)
    {
        MY_VERIFY(pItem);
        m_cQueueProduced.Enqueue(pItem);
        pItem = NULL;
    };

    inline
    ItemType* getItemForConsume()
    {
        ItemType *pItem = NULL;
        m_cQueueProduced.Dequeue(pItem);
        return pItem;
    };

    inline
    void putInConsumedItem(ItemType* &pItem)
    {
        MY_VERIFY(pItem);
        m_cQueueConsumed.Enqueue(pItem);
        pItem = NULL;
    };

protected:
    TQueue<ItemType*, Mode> m_cQueueProduced;
    TQueue<ItemType*, Mode> m_cQueueConsumed;

    uint32 m_uiBufferCount;

    bool m_bDebugLackBufferForProduce;
    FString m_sDebugName;
};

/*
template<EQueueMode Mode>
void FMyQueueWithLimitBuffer<UObject, Mode>::checkType()
{
    UE_MY_LOG(LogMyUtilsInstance, Error, TEXT("we don't support UObject* as itemType yet!"));
    MY_VERIFY(false);
};

template<typename ItemType, EQueueMode Mode>
void FMyQueueWithLimitBuffer<ItemType, Mode>::checkType()
{

};

template<typename T, typename U>
class Conversion
{
private:
typedef char Small;
struct Big{ char big[2]; };

static Small _helper_fun(U);
static Big _helper_fun(...);
static T _make_T();
public:
enum {
Exists = (sizeof(_helper_fun(_make_T())) == sizeof(Small)),
Exists2Way = ( Exists && Conversion<U,T>::Exists),
Same = false
};
};
*/


UCLASS()
class UMyCommonUtilsLibrary :
    public UBlueprintFunctionLibrary
{
    GENERATED_BODY()

public:

    inline
    static FString genTimeStrFromTimeMs(uint32 uiTime_ms)
    {
        return FString::Printf(TEXT("%u.%03u"), uiTime_ms / 1000, uiTime_ms % 1000);
    };
};