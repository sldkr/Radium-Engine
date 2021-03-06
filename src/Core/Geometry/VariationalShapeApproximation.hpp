#pragma once

#include <Core/Container/VectorArray.hpp>
#include <Core/Geometry/TriangleMesh.hpp>
#include <array>
#include <queue>
#include <vector>

namespace Ra {
namespace Core {
namespace Geometry {

/**
 * @brief The MetricType enum
 *
 *        The MetricType enum is used to select the type of metric for the VSA algorithm.
 */
enum class MetricType { L2, L21, LLOYD };

/**
 * @brief The VariationalShapeApproximationBase class
 *
 *        The VariationalShapeApproximationBase class computes the K proxies describing
 *        a surface from a given triangle mesh.
 *
 * @note It is based on "Variational Shape Approximation" paper.
 * @warning It doesn't implement region teleporting or non-organic shape partitioning.
 */
template <uint K_Region>
class VariationalShapeApproximationBase {
    static_assert( ( K_Region > 0 ), "K_Region must be greater than 0" );

  public:
    //////////////////////////////////////////////////////////////////////////////
    // CONSTANT
    //////////////////////////////////////////////////////////////////////////////
    static constexpr uint K = K_Region; ///< The number of regions to have.

    //////////////////////////////////////////////////////////////////////////////
    // TYPEDEF
    //////////////////////////////////////////////////////////////////////////////
    using Mesh = TriangleMesh; ///< Mesh class.

    using FaceBarycenter = Container::Vector3Array;   ///< Face barycenters.
    using FaceNormal = Container::Vector3Array;       ///< Face normals.
    using FaceArea = std::vector<Scalar>;  ///< Face areas.
    using FaceRegion = std::vector<uint>;  ///< Face region IDs.
    using FaceVisited = std::vector<bool>; ///< Dirty bits
    using FaceValue = std::vector<Scalar>; ///< Face value containing the color value.
    using FaceAdjacency = std::vector<std::set<TriangleIdx>>;

    using Proxy = std::pair<Vector3, Vector3>;      ///< Proxy structure.
    using ProxyList = std::array<Proxy, K>;         ///< List of proxies.
    using Region = std::vector<TriangleIdx>;        ///< Region structure.
    using RegionList = std::array<Region, K>;       ///< List of regions.
    using Energy = Scalar;                          ///< Energy value.
    using Pair = std::pair<TriangleIdx, uint>;      ///< Triangle-Proxy pair.
    using QueueEntry = std::pair<Energy, Pair>;     ///< Queue entry. It contains < Energy, <T,P> >.
    using QueueEntryList = std::vector<QueueEntry>; ///< List of queue entries.

    using PriorityQueue =
        std::priority_queue<QueueEntry, QueueEntryList,
                            std::greater<QueueEntryList::value_type>>; ///< Priority queue.

    //////////////////////////////////////////////////////////////////////////////
    // CONSTRUCTOR
    //////////////////////////////////////////////////////////////////////////////
    explicit inline VariationalShapeApproximationBase( const Mesh& mesh );
    inline VariationalShapeApproximationBase( const VariationalShapeApproximationBase& other ) =
        default;
    inline VariationalShapeApproximationBase( VariationalShapeApproximationBase&& other ) = default;

    //////////////////////////////////////////////////////////////////////////////
    // DESTRUCTOR
    //////////////////////////////////////////////////////////////////////////////
    inline ~VariationalShapeApproximationBase();

    //////////////////////////////////////////////////////////////////////////////
    // INIT
    //////////////////////////////////////////////////////////////////////////////
    inline void init();              ///< Initialize the data and set the seed triangles.
    inline bool initialized() const; ///< Return true if init() was called. False otherwise.

    //////////////////////////////////////////////////////////////////////////////
    // EXECUTION
    //////////////////////////////////////////////////////////////////////////////
    inline void
    exec( const uint iteration =
              20 ); ///< Execute the algorithm performing the given amount of iterations.

    template <uint Iteration>
    inline void exec(); ///< Execute the algorithm performing the given amount of iterations.

    //////////////////////////////////////////////////////////////////////////////
    // REGION
    //////////////////////////////////////////////////////////////////////////////
    inline const Region& region( const uint i ) const; ///< Returns the i-th region.

    //////////////////////////////////////////////////////////////////////////////
    // PROXY
    //////////////////////////////////////////////////////////////////////////////
    inline const Proxy& proxy( const uint i ) const; ///< Returns the i-th proxy.

    //////////////////////////////////////////////////////////////////////////////
    // COLOR
    //////////////////////////////////////////////////////////////////////////////
    inline void create_region_color(); ///< Create the colors values for the faces.
    inline void create_energy_color(); ///< Create the colors values for the faces.
    inline void shuffle_regions(); ///< Shuffle the regions in order to have less color conflicts.

  protected:
    //////////////////////////////////////////////////////////////////////////////
    // DATA
    //////////////////////////////////////////////////////////////////////////////
    inline void compute_data(); ///< Initialize the mesh properties and the algorithm data.

    //////////////////////////////////////////////////////////////////////////////
    // SEED
    //////////////////////////////////////////////////////////////////////////////
    inline void compute_seed(); ///< Computes the seed triangles in a randomized fashion way.

    //////////////////////////////////////////////////////////////////////////////
    // GEOMETRY PARTITIONING
    //////////////////////////////////////////////////////////////////////////////
    inline void
    geometry_partitioning(); ///< Performs the geometry partitioning with the current proxies.
    inline void add_neighbors_to_queue(
        const TriangleIdx& T,
        const uint proxy_id ); ///< Add the neighbors of T to the priority queue.

    //////////////////////////////////////////////////////////////////////////////
    // INTERFACE
    //////////////////////////////////////////////////////////////////////////////
    virtual void proxy_fitting() = 0; ///< Performs the Proxy fitting.
    virtual Energy E( const TriangleIdx& T, const Proxy& P )
        const = 0; ///< Computes the energy function for the given triangle and the given Proxy.

  protected:
    //////////////////////////////////////////////////////////////////////////////
    // VARIABLE
    //////////////////////////////////////////////////////////////////////////////
    const Mesh& m_mesh;

    FaceBarycenter m_fbary;
    FaceNormal m_fnormal;
    FaceArea m_farea;
    FaceRegion m_fregion;
    FaceVisited m_fvisited;
    FaceValue m_fvalue;
    FaceAdjacency m_fadj;

    PriorityQueue m_queue;
    RegionList m_region;
    ProxyList m_proxy;

    bool m_init;
};

//============================================================================
//============================================================================
//============================================================================

/**
 * @brief The VariationalShapeApproximation class
 *
 *        The VariationalShapeApproximation class implements the proxy fitting
 *        accordingly to the selected metric.
 */
template <uint K_Region, MetricType Type = MetricType::L2>
class VariationalShapeApproximation : public VariationalShapeApproximationBase<K_Region> {
  public:
    //////////////////////////////////////////////////////////////////////////////
    // TYPEDEF
    //////////////////////////////////////////////////////////////////////////////
    // using Triangle = typename VariationalShapeApproximationBase< K_Region >::Triangle;
    using Proxy = typename VariationalShapeApproximationBase<K_Region>::Proxy;

    //////////////////////////////////////////////////////////////////////////////
    // CONSTRUCTOR
    //////////////////////////////////////////////////////////////////////////////
    using VariationalShapeApproximationBase<K_Region>::VariationalShapeApproximationBase;

    //////////////////////////////////////////////////////////////////////////////
    // DESTRUCTOR
    //////////////////////////////////////////////////////////////////////////////
    virtual ~VariationalShapeApproximation();

  protected:
    //////////////////////////////////////////////////////////////////////////////
    // PROXY FITTING
    //////////////////////////////////////////////////////////////////////////////
    inline void proxy_fitting() override final; ///< Computes the Proxy fitting for L2 metric.

    //////////////////////////////////////////////////////////////////////////////
    // ENERGY
    //////////////////////////////////////////////////////////////////////////////
    inline Scalar
    E( const TriangleIdx& T,
       const Proxy& P ) const override final; ///< Computes the energy function for the L2 metric.
};

//============================================================================
//============================================================================
//============================================================================

/**
 * @brief This is a specialized version of the VSA for the L21 metric.
 */
template <uint K_Region>
class VariationalShapeApproximation<K_Region, MetricType::L21>
    : public VariationalShapeApproximationBase<K_Region> {
  public:
    //////////////////////////////////////////////////////////////////////////////
    // TYPEDEF
    //////////////////////////////////////////////////////////////////////////////
    // using  Triangle = typename VariationalShapeApproximationBase< K_Region >::Triangle;
    using Proxy = typename VariationalShapeApproximationBase<K_Region>::Proxy;

    //////////////////////////////////////////////////////////////////////////////
    // CONSTRUCTOR
    //////////////////////////////////////////////////////////////////////////////
    using VariationalShapeApproximationBase<K_Region>::VariationalShapeApproximationBase;

    //////////////////////////////////////////////////////////////////////////////
    // DESTRUCTOR
    //////////////////////////////////////////////////////////////////////////////
    virtual ~VariationalShapeApproximation();

  protected:
    //////////////////////////////////////////////////////////////////////////////
    // PROXY FITTING
    //////////////////////////////////////////////////////////////////////////////
    inline void proxy_fitting() override final; ///< Computes the Proxy fitting for the L21 metric.

    //////////////////////////////////////////////////////////////////////////////
    // ENERGY
    //////////////////////////////////////////////////////////////////////////////
    inline Scalar
    E( const TriangleIdx& T,
       const Proxy& P ) const override final; ///< Computes the energy function for the L21 metric.
};

//============================================================================
//============================================================================
//============================================================================

/**
 * @brief This is a specialized version of the VSA for the L21 metric.
 */
template <uint K_Region>
class VariationalShapeApproximation<K_Region, MetricType::LLOYD>
    : public VariationalShapeApproximationBase<K_Region> {
  public:
    //////////////////////////////////////////////////////////////////////////////
    // TYPEDEF
    //////////////////////////////////////////////////////////////////////////////
    // using  Triangle = typename VariationalShapeApproximationBase< K_Region >::Triangle ;
    using Proxy = typename VariationalShapeApproximationBase<K_Region>::Proxy;

    //////////////////////////////////////////////////////////////////////////////
    // CONSTRUCTOR
    //////////////////////////////////////////////////////////////////////////////
    using VariationalShapeApproximationBase<K_Region>::VariationalShapeApproximationBase;

    //////////////////////////////////////////////////////////////////////////////
    // DESTRUCTOR
    //////////////////////////////////////////////////////////////////////////////
    virtual ~VariationalShapeApproximation();

  protected:
    //////////////////////////////////////////////////////////////////////////////
    // PROXY FITTING
    //////////////////////////////////////////////////////////////////////////////
    inline void proxy_fitting() override final; ///< Computes the Proxy fitting for the L21 metric.

    //////////////////////////////////////////////////////////////////////////////
    // ENERGY
    //////////////////////////////////////////////////////////////////////////////
    inline Scalar
    E( const TriangleIdx& T,
       const Proxy& P ) const override final; ///< Computes the energy function for the L21 metric.
};

//============================================================================
//============================================================================
//============================================================================

//////////////////////////////////////////////////////////////////////////////
// ALIAS
//////////////////////////////////////////////////////////////////////////////
template <uint K_Region, MetricType Type>
using VSA = VariationalShapeApproximation<K_Region, Type>;

template <uint K_Region>
using VSA_L2 = VariationalShapeApproximation<K_Region, MetricType::L2>;

template <uint K_Region>
using VSA_L21 = VariationalShapeApproximation<K_Region, MetricType::L21>;

template <uint K_Region>
using VSA_L21 = VariationalShapeApproximation<K_Region, MetricType::LLOYD>;

} // namespace Geometry
} // namespace Core
} // namespace Ra

#include <Core/Geometry/Approximation/VariationalShapeApproximation.inl>
