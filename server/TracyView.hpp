#ifndef __TRACYVIEW_HPP__
#define __TRACYVIEW_HPP__

#include <atomic>
#include <mutex>
#include <string>
#include <thread>
#include <unordered_map>
#include <unordered_set>
#include <vector>

#include "../common/tracy_lz4.hpp"
#include "../common/TracySocket.hpp"
#include "../common/TracyQueue.hpp"
#include "TracyEvent.hpp"
#include "TracySlab.hpp"
#include "TracySourceLocation.hpp"
#include "TracyVector.hpp"

struct ImVec2;

namespace tracy
{

struct QueueItem;

class View
{
public:
    View() : View( "127.0.0.1" ) {}
    View( const char* addr );
    ~View();

    static bool ShouldExit();
    static void Draw();

private:
    struct ThreadData
    {
        uint64_t id;
        Vector<Event*> timeline;
    };

    void Worker();

    void DispatchProcess( const QueueItem& ev );
    void DispatchProcess( const QueueItem& ev, const char*& ptr );

    void Process( const QueueItem& ev );
    void ProcessZoneBegin( uint64_t id, const QueueZoneBegin& ev );
    void ProcessZoneEnd( uint64_t id, const QueueZoneEnd& ev );
    void ProcessFrameMark( uint64_t id );

    void CheckString( uint64_t ptr );
    void CheckThreadString( uint64_t id );
    void AddString( uint64_t ptr, std::string&& str );
    void AddThreadString( uint64_t id, std::string&& str );

    void NewZone( Event* zone, uint64_t thread );
    void UpdateZone( Event* zone );

    void InsertZone( Event* zone, Event* parent, Vector<Event*>& vec );

    uint64_t GetFrameTime( size_t idx ) const;
    uint64_t GetFrameBegin( size_t idx ) const;
    uint64_t GetFrameEnd( size_t idx ) const;
    uint64_t GetLastTime() const;
    uint64_t GetZoneEnd( const Event& ev ) const;
    Vector<Event*>& GetParentVector( const Event& ev );
    const char* TimeToString( uint64_t ns ) const;
    const char* GetString( uint64_t ptr ) const;
    const char* GetThreadString( uint64_t id ) const;

    void DrawImpl();
    void DrawFrames();
    void DrawZones();
    int DrawZoneLevel( const Vector<Event*>& vec, bool hover, double pxns, const ImVec2& wpos, int offset, int depth );

    std::string m_addr;

    Socket m_sock;
    std::thread m_thread;
    std::atomic<bool> m_shutdown;
    std::atomic<bool> m_connected;
    std::atomic<bool> m_hasData;

    // this block must be locked
    std::mutex m_lock;
    Vector<uint64_t> m_frames;
    Vector<SourceLocation> m_srcFile;
    Vector<ThreadData> m_threads;
    std::unordered_map<uint64_t, std::string> m_strings;
    std::unordered_map<uint64_t, std::string> m_threadNames;

    std::mutex m_mbpslock;
    std::vector<float> m_mbps;

    // not used for vis - no need to lock
    std::unordered_map<uint64_t, QueueZoneEnd> m_pendingEndZone;
    std::unordered_map<uint64_t, Event*> m_openZones;
    std::unordered_set<uint64_t> m_pendingStrings;
    std::unordered_set<uint64_t> m_pendingThreads;
    std::unordered_map<SourceLocation, uint32_t, SourceLocation::Hasher, SourceLocation::Comparator> m_locationRef;
    std::unordered_map<uint64_t, uint32_t> m_threadMap;

    Slab<EventSize*1024*1024> m_slab;

    LZ4_streamDecode_t* m_stream;
    char* m_buffer;
    int m_bufferOffset;

    int m_frameScale;
    bool m_pause;
    int m_frameStart;

    int64_t m_zvStart;
    int64_t m_zvEnd;
};

}

#endif
