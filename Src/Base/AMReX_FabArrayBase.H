#ifndef BL_FABARRAYBASE_H_
#define BL_FABARRAYBASE_H_

#ifdef _OPENMP
#include <omp.h>
#endif

#ifdef BL_USE_UPCXX
#include <AMReX_BLPgas.H>
#endif

#include <AMReX_BoxArray.H>
#include <AMReX_DistributionMapping.H>
#include <AMReX_ParallelDescriptor.H>
#include <AMReX_Periodicity.H>
#include <AMReX_Print.H>

namespace amrex {

class MFIter;
class MFGhostIter;
class Geometry;

class FabArrayBase
{
    friend class MFIter;
    friend class MFGhostIter;

public:

    FabArrayBase ();

    virtual ~FabArrayBase();

    FabArrayBase (FabArrayBase&& rhs) noexcept = default;
    
    FabArrayBase (const FabArrayBase& rhs) = delete;
    FabArrayBase& operator= (const FabArrayBase& rhs) = delete;
    FabArrayBase& operator= (FabArrayBase&& rhs) = delete;    

    void define (const BoxArray&            bxs,
                 const DistributionMapping& dm,
                 int                        nvar,
                 int                        ngrow);

    //! Return the grow factor that defines the region of definition.
    int nGrow () const { return n_grow; }

    //! Return number of variables (aka components) associated with each point.
    int nComp () const { return n_comp; }

    //! Return index type.
    IndexType ixType () const { return boxarray.ixType(); }
    
    //! Return the communicator associated with this FabArray
    ParallelDescriptor::Color color () const { return distributionMap.color(); }

    //Return whether this FabArray is empty
    bool empty () const { return boxarray.empty(); }

    /**
    * \brief Return a constant reference to the BoxArray that defines the
    * valid region associated with this FabArray.
    */
    const BoxArray& boxArray () const { return boxarray; }

    /**
    * \brief Return the Kth Box in the BoxArray.
    * That is, the valid region of the Kth grid.
    */
    Box box (int K) const { return boxarray[K]; }

    /**
    * \brief Return the Kth FABs Box in the FabArray.
    * That is, the region the Kth fab is actually defined on.
    */
    Box fabbox (int K) const;

    //! Return the number of FABs in the FabArray.
    int size () const { return boxarray.size(); }

    //! Return the number of local FABs in the FabArray.
    int local_size () const { return indexArray.size(); }

    //! Return constant reference to indices in the FabArray that we have access.
    const Array<int> &IndexArray () const { return indexArray; }

    //! Return local index in the vector of FABs.
    int localindex (int K) const { 
        std::vector<int>::const_iterator low
            = std::lower_bound(indexArray.begin(), indexArray.end(), K);
        if (low != indexArray.end() && *low == K) {
            return low - indexArray.begin();
        }
        else {
            return -1;
        }
    }

    //! Return constant reference to associated DistributionMapping.
    const DistributionMapping& DistributionMap () const { return distributionMap; }

    //
    struct CacheStats
    {
	int         size;     // current size: nbuild - nerase
	int         maxsize;  // highest water mark of size
	int         maxuse;   // max # of uses of a cached item
	long        nuse;     // # of uses of the whole cache
	long        nbuild;   // # of build operations
	long        nerase;   // # of erase operations
	long        bytes;
	long        bytes_hwm;
	std::string name;     // name of the cache
	CacheStats (const std::string& name_) 
	    : size(0),maxsize(0),maxuse(0),nuse(0),nbuild(0),nerase(0),
	      bytes(0L),bytes_hwm(0L),name(name_) {;}
	void recordBuild () {
	    ++size;  
	    ++nbuild;  
	    maxsize = std::max(maxsize, size); 
	}
	void recordErase (int n) { 
	    // n: how many times the item to be deleted has been used.
	    --size;
	    ++nerase;
	    maxuse = std::max(maxuse, n);
	}
	void recordUse () { ++nuse; }
	void print () {
	    amrex::Print(Print::AllProcs) << "### " << name << " ###\n"
					  << "    tot # of builds  : " << nbuild  << "\n"
					  << "    tot # of erasures: " << nerase  << "\n"
					  << "    tot # of uses    : " << nuse    << "\n"
					  << "    max cache size   : " << maxsize << "\n"
					  << "    max # of uses    : " << maxuse  << "\n";
	}
    };
    //
    // Used by a bunch of routines when communicating via MPI.
    //
    struct CopyComTag
    {
	Box dbox;
        Box sbox;
        int dstIndex;
        int srcIndex;
	CopyComTag () {}
	CopyComTag (const Box& db, const Box& sb, int didx, int sidx)
	    : dbox(db), sbox(sb), dstIndex(didx), srcIndex(sidx) {}
	bool operator< (const CopyComTag& rhs) const {
	    return (srcIndex < rhs.srcIndex) || ((srcIndex == rhs.srcIndex) && (
                   (dstIndex < rhs.dstIndex) || ((dstIndex == rhs.dstIndex) && (
                   (dbox.smallEnd() < rhs.dbox.smallEnd()
		               || ((dbox.smallEnd() == rhs.dbox.smallEnd()) && (
                   (sbox.smallEnd() < rhs.sbox.smallEnd()))))))));
	}
        //
        // Some typedefs & helper functions used throughout the code.
        //
        typedef std::vector<CopyComTag> CopyComTagsContainer;

        typedef std::map<int,CopyComTagsContainer> MapOfCopyComTagContainers;
    };
    //
    // Some useful typedefs.
    //
    typedef CopyComTag::CopyComTagsContainer CopyComTagsContainer;
    typedef CopyComTag::MapOfCopyComTagContainers MapOfCopyComTagContainers;
    //
    static long bytesOfMapOfCopyComTagContainers (const MapOfCopyComTagContainers&);

    // Key for unique combination of BoxArray and DistributionMapping
    // Note both BoxArray and DistributionMapping are reference counted.
    // Objects with the same references have the same key.
    struct BDKey {
        BDKey () = default;
        BDKey (const BoxArray::RefID& baid, const DistributionMapping::RefID& dmid)
            : m_ba_id(baid), m_dm_id(dmid) {}
        bool operator< (const BDKey& rhs) const {
            return (m_ba_id < rhs.m_ba_id) ||
                ((m_ba_id == rhs.m_ba_id) && (m_dm_id < rhs.m_dm_id));
        }
        bool operator== (const BDKey& rhs) const {
            return m_ba_id == rhs.m_ba_id && m_dm_id == rhs.m_dm_id;
        }
        bool operator!= (const BDKey& rhs) const {
            return m_ba_id != rhs.m_ba_id || m_dm_id != rhs.m_dm_id;
        }
    private:
        BoxArray::RefID            m_ba_id;
        DistributionMapping::RefID m_dm_id;
    };

    BDKey getBDKey () const {
	return {boxarray.getRefID(), distributionMap.getRefID()};
    }

    void updateBDKey ();

    //
    // Tiling
    //
    struct TileArray
    {
	int nuse;
	Array<int> numLocalTiles;
	Array<int> indexMap;	
	Array<int> localIndexMap;
	Array<int> localTileIndexMap;
	Array<Box> tileArray;
	TileArray () : nuse(-1) {;}
	long bytes () const;
    };

    //
    // Used for collecting information used in communicating FABs.
    //
    struct FabComTag
    {
        int fromProc;
        int toProc;
        int fabIndex;
        int fineIndex;
        int srcComp;
        int destComp;
        int nComp;
        int face;
        int fabArrayId;
        int fillBoxId;
        int procThatNeedsData;
        int procThatHasData;
        Box box;

        FabComTag ()
            :
            fromProc(0),
            toProc(0),
            fabIndex(0),
            fineIndex(0),
            srcComp(0),
            destComp(0),
            nComp(0),
            face(0),
            fabArrayId(0),
            fillBoxId(0),
            procThatNeedsData(0),
            procThatHasData(0) {}
    };
    //
    // Default tilesize in MFIter
    //
    static IntVect mfiter_tile_size;
    //
    // Default tilesize in MFGhostIter
    //
    static IntVect mfghostiter_tile_size;
    //
    // The maximum number of components to copy() at a time.
    //
    static int MaxComp;
    //
    // Use MPI_Asend() instead of MPI_Send() in CollectData() and copy().
    //
    // Turn off via ParmParse using "fabarray.do_async_sends=0" in inputs file.
    //
    // Default is true.
    //
    static bool do_async_sends;
    //
    // Initialize from ParmParse with "fabarray" prefix.
    //
    static void Initialize ();
    static void Finalize ();
    bool IsInitialized() const;
    void SetInitialized(bool binit);
    //
    // To maximize thread efficiency we now can decompose things like
    // intersections among boxes into smaller tiles. This sets
    // their maximum size.
    //
    static IntVect comm_tile_size;  // communication tile size
    //
    // The number of FabArrays that have been defined during this run
    //
    static int nFabArrays;
    static int NFabArrays() { return nFabArrays; }
    int AllocatedFAPtrID() const  { return aFAPId; }

    struct FPinfo
    {
	FPinfo (const FabArrayBase& srcfa,
		const FabArrayBase& dstfa,
		Box                 dstdomain,
		int                 dstng,
		const BoxConverter& coarsener);
	~FPinfo ();

	long bytes () const;

	BoxArray            ba_crse_patch;
	DistributionMapping dm_crse_patch;
	Array<int>          dst_idxs;
	Array<Box>          dst_boxes;
	//
	BDKey               m_srcbdk;
	BDKey               m_dstbdk;
	Box                 m_dstdomain;
	int                 m_dstng;
	BoxConverter*       m_coarsener;
	//
	int                 m_nuse;
    };

    typedef std::multimap<BDKey,FabArrayBase::FPinfo*> FPinfoCache;
    typedef FPinfoCache::iterator FPinfoCacheIter;

    static FPinfoCache m_TheFillPatchCache;

    static CacheStats m_FPinfo_stats;

    static const FPinfo& TheFPinfo (const FabArrayBase& srcfa,
				    const FabArrayBase& dstfa,
				    Box                 dstdomain,
				    int                 dstng,
				    const BoxConverter& coarsener);

    void flushFPinfo (bool no_assertion=false);

    //
    // coarse/fine boundary
    //
    struct CFinfo
    {
        CFinfo (const FabArrayBase& finefa,
                const Geometry&     finegm,
                int                 ng,
                bool                include_periodic,
                bool                include_physbndry);

        long bytes () const;

        static Box Domain (const Geometry& geom, int ng,
                           bool include_periodic, bool include_physbndry);

        BoxArray            ba_cfb;
        DistributionMapping dm_cfb;
        Array<int>          fine_grid_idx; // local array
        //
        BDKey               m_fine_bdk;
        Box                 m_fine_domain;
        int                 m_ng;
        bool                m_include_periodic;
        bool                m_include_physbndry;
        //
        int                 m_nuse;
    };

    using CFinfoCache = std::multimap<BDKey,FabArrayBase::CFinfo*>;
    using CFinfoCacheIter = CFinfoCache::iterator;

    static CFinfoCache m_TheCrseFineCache;

    static CacheStats m_CFinfo_stats;

    static const CFinfo& TheCFinfo (const FabArrayBase& finefa,
                                    const Geometry&     finegm,
                                    int                 ng,
                                    bool                include_periodic,
                                    bool                include_physbndry);

    void flushCFinfo (bool no_assertion=false);

    //
    // parallel copy or add
    //
    enum CpOp { COPY = 0, ADD = 1 };

    const TileArray* getTileArray (const IntVect& tilesize) const;

    //! Block until all send requests complete
    static void WaitForAsyncSends (int                 N_snds,
                                   Array<MPI_Request>& send_reqs,
                                   Array<char*>&       send_data,
                                   Array<MPI_Status>&  stats);

#ifdef BL_USE_UPCXX
    //! Block until all send requests complete
    static void WaitForAsyncSends_PGAS (int                 N_snds,
                                        Array<char*>&       send_data,
                                        upcxx::event*       send_event,
                                        volatile int*       send_counter);
#endif

protected:

    DistributionMapping& ModifyDistributionMap () { return distributionMap; }

    /**
    * \brief Return owenership of fabs. The concept of ownership only applies when UPC++
    * team is used. In that case, each fab is shared by team workers, with one
    * taking the ownership.
    */
    const std::vector<bool>& OwnerShip () const { return ownership; }
    bool isOwner (int li) const { return ownership[li]; }

    //
    // The data ...
    //
    mutable BoxArray    boxarray;
    DistributionMapping distributionMap;
    Array<int>          indexArray;
    std::vector<bool>   ownership;
    int                 n_grow;
    int                 n_comp;
    int                 aFAPId;      // ---- id of currently allocated fab array pointers
    int                 aFAPIdLock;  // ---- lock for resizing sidecars

    mutable BDKey m_bdkey;

    //
    // Tiling
    //
    // We use tile size as the key for the inner map.
    
    using TAMap   = std::map<std::pair<IntVect,IntVect>, TileArray>;
    using TACache = std::map<BDKey, TAMap>;
    //
    static TACache     m_TheTileArrayCache;
    static CacheStats  m_TAC_stats;
    //
    void buildTileArray (const IntVect& tilesize, TileArray& ta) const;
    //
    void flushTileArray (const IntVect& tilesize = IntVect::TheZeroVector(), 
			 bool no_assertion=false) const;
    static void flushTileArrayCache (); // This flushes the entire cache.

    //
    // FillBoundary
    //
    struct FB
    {
        FB (const FabArrayBase& fa, bool cross, const Periodicity& period,
	    bool enforce_periodicity_only);
        ~FB ();

	IndexType    m_typ;
        IntVect      m_crse_ratio; // BoxArray in FabArrayBase may have crse_ratio.
        int          m_ngrow;
        bool         m_cross;
	bool         m_epo;
	Periodicity  m_period;
        //
        // The cache of local and send/recv per FillBoundary().
        //
	bool                m_threadsafe_loc;
	bool                m_threadsafe_rcv;
        CopyComTagsContainer*      m_LocTags;
        MapOfCopyComTagContainers* m_SndTags;
        MapOfCopyComTagContainers* m_RcvTags;
        MapOfCopyComTagContainers* m_SndVols;
        MapOfCopyComTagContainers* m_RcvVols;
	//
	int                 m_nuse;
	//
	long bytes () const;
    private:
	void define_fb (const FabArrayBase& fa);
	void define_epo (const FabArrayBase& fa);
    };
    //
    typedef std::multimap<BDKey,FabArrayBase::FB*> FBCache;
    typedef FBCache::iterator FBCacheIter;
    //
    static FBCache    m_TheFBCache;
    static CacheStats m_FBC_stats;
    //
    const FB& getFB (const Periodicity& period, bool cross=false, bool enforce_periodicity_only = false) const;
    //
    void flushFB (bool no_assertion=false) const;       // This flushes its own FB.
    static void flushFBCache (); // This flushes the entire cache.

    //
    // parallel copy or add
    //
    struct CPC
    {
	CPC (const FabArrayBase& dstfa, int dstng,
	     const FabArrayBase& srcfa, int srcng,
	     const Periodicity& period);
	CPC (const BoxArray& dstba, const DistributionMapping& dstdm, 
	     const Array<int>& dstidx, int dstng,
	     const BoxArray& srcba, const DistributionMapping& srcdm, 
	     const Array<int>& srcidx, int srcng,
	     const Periodicity& period, int myproc);
        ~CPC ();

        long bytes () const;	

	BDKey       m_srcbdk;
	BDKey       m_dstbdk;
	int         m_srcng;
	int         m_dstng;
	Periodicity m_period;
	BoxArray    m_srcba;
	BoxArray    m_dstba;
        //
        // The cache of local and send/recv info per FabArray::copy().
        //
	bool        m_threadsafe_loc;
	bool        m_threadsafe_rcv;
        CopyComTagsContainer*      m_LocTags;
        MapOfCopyComTagContainers* m_SndTags;
        MapOfCopyComTagContainers* m_RcvTags;
        MapOfCopyComTagContainers* m_SndVols;
        MapOfCopyComTagContainers* m_RcvVols;
	//
        int         m_nuse;

    private:
	void define (const BoxArray& ba_dst, const DistributionMapping& dm_dst,
		     const Array<int>& imap_dst,
		     const BoxArray& ba_src, const DistributionMapping& dm_src,
		     const Array<int>& imap_src,
		     int MyProc = ParallelDescriptor::MyProc());
    };
    //
    typedef std::multimap<BDKey,FabArrayBase::CPC*> CPCache;
    typedef CPCache::iterator CPCacheIter;
    //
    static CPCache    m_TheCPCache;
    static CacheStats m_CPC_stats;
    //
    const CPC& getCPC (int dstng, const FabArrayBase& src, int srcng, const Periodicity& period) const;
    // 
    void flushCPC (bool no_assertion=false) const;      // This flushes its own CPC.
    static void flushCPCache (); // This flusheds the entire cache.

    //
    // Keep track of how many FabArrays are built with the same BDKey.
    //
    static std::map<BDKey, int> m_BD_count;
    //
    // clear BD count and caches associated with this BD, if no other is using this BD. 
    // 
    void clearThisBD (bool no_assertion=false);
    //
    // add the current BD into BD count database
    //
    void addThisBD ();
    //
    struct FabArrayStats
    {
	int  num_fabarrays;
	int  max_num_fabarrays;
	int  max_num_boxarrays;
	int  max_num_ba_use;
	long num_build;
	FabArrayStats () : num_fabarrays(0), max_num_fabarrays(0), max_num_boxarrays(0),
			   max_num_ba_use(1), num_build(0) {;}
	void recordBuild () {
	    ++num_fabarrays;
	    ++num_build;
	    max_num_fabarrays = std::max(max_num_fabarrays, num_fabarrays);
	}
	void recordDelete () {
	    --num_fabarrays;
	}
	void recordMaxNumBoxArrays (int n) {
	    max_num_boxarrays = std::max(max_num_boxarrays, n);
	}
	void recordMaxNumBAUse (int n) {
	    max_num_ba_use = std::max(max_num_ba_use, n);
	}
	void print () {
	    amrex::Print(Print::AllProcs) << "### FabArray ###\n"
					  << "    tot # of builds       : " << num_build         << "\n"
					  << "    max # of FabArrays    : " << max_num_fabarrays << "\n"
					  << "    max # of BoxArrays    : " << max_num_boxarrays << "\n"
					  << "    max # of BoxArray uses: " << max_num_ba_use    << "\n";
	}
    };
    static FabArrayStats m_FA_stats;

#ifdef BL_USE_MPI
    static bool CheckRcvStats(const Array<MPI_Status>& recv_stats,
			      const Array<int>& recv_size,
			      MPI_Datatype datatype, int tag);
#endif
};

}

#endif
