#include "ProgressiveMesh.hpp"

#include <Core/RaCore.hpp>

#include <Core/Log/Log.hpp>

#include <Core/Mesh/Wrapper/Convert.hpp>

#include <Core/Mesh/DCEL/Vertex.hpp>
#include <Core/Mesh/DCEL/HalfEdge.hpp>
#include <Core/Mesh/DCEL/FullEdge.hpp>
//#include <Core/Mesh/DCEL/Dcel.hpp>
#include <Core/Mesh/DCEL/Operations/EdgeCollapse.hpp>
#include <Core/Mesh/DCEL/Operations/VertexSplit.hpp>
#include <Core/Geometry/Triangle/TriangleOperation.hpp>

#include <Core/Mesh/DCEL/Iterator/Vertex/VVIterator.hpp>
#include <Core/Mesh/DCEL/Iterator/Vertex/VFIterator.hpp>
#include <Core/Mesh/DCEL/Iterator/Vertex/VHEIterator.hpp>
#include <Core/Mesh/DCEL/Iterator/Edge/EFIterator.hpp>

#include <Core/Mesh/ProgressiveMesh/PriorityQueue.hpp>

#include <Core/Geometry/Triangle/TriangleOperation.hpp>

namespace Ra
{
    namespace Core
    {

        template<class ErrorMetric>
        ProgressiveMesh<ErrorMetric>::ProgressiveMesh(TriangleMesh* mesh)
        {
            m_dcel = new Dcel();
            m_em = ErrorMetric(100.0);
            m_nb_faces = mesh->m_triangles.size();
            m_nb_vertices = mesh->m_vertices.size();

            convert(*mesh, *m_dcel);

            m_bbox_size = computeBoundingBoxSize();
            m_mean_edge_size = MeshUtils::getMeanEdgeLength(*mesh);
            m_scale = 0.0;
        }

        //------------------------------------------------

        template <class ErrorMetric>
        inline Dcel* ProgressiveMesh<ErrorMetric>::getDcel()
        {
            return m_dcel;
        }

        template <class ErrorMetric>
        inline int ProgressiveMesh<ErrorMetric>::getNbFaces()
        {
            return m_nb_faces;
        }

        template <class ErrorMetric>
        inline ErrorMetric ProgressiveMesh<ErrorMetric>::getEM()
        {
            return m_em;
        }

        //------------------------------------------------

        template <class ErrorMetric>
        Scalar ProgressiveMesh<ErrorMetric>::computeBoundingBoxSize()
        {
            Scalar min_x, max_x, min_y, max_y, min_z, max_z;
            min_x = max_x = m_dcel->m_vertex[0]->P().x();
            min_y = max_y = m_dcel->m_vertex[0]->P().y();
            min_z = max_z = m_dcel->m_vertex[0]->P().z();
            for (int i = 0; i < m_dcel->m_vertex.size(); i++)
            {
                if (m_dcel->m_vertex[i]->P().x() < min_x) min_x = m_dcel->m_vertex[i]->P().x();
                if (m_dcel->m_vertex[i]->P().x() > max_x) max_x = m_dcel->m_vertex[i]->P().x();
                if (m_dcel->m_vertex[i]->P().y() < min_y) min_y = m_dcel->m_vertex[i]->P().y();
                if (m_dcel->m_vertex[i]->P().y() > max_y) max_y = m_dcel->m_vertex[i]->P().y();
                if (m_dcel->m_vertex[i]->P().z() < min_z) min_z = m_dcel->m_vertex[i]->P().z();
                if (m_dcel->m_vertex[i]->P().z() > max_z) max_z = m_dcel->m_vertex[i]->P().z();
            }
            Vector3 size = Vector3(max_x-min_x, max_y-min_y, max_z-min_z);
            //Vector3 center = Vector3((min_x+max_x)/2, (min_y+max_y)/2, (min_z+max_z)/2);
            return size.norm();
        }

        //------------------------------------------------

        template <class ErrorMetric>
        void ProgressiveMesh<ErrorMetric>::computeFacesQuadrics()
        {
            const uint numTriangles = m_dcel->m_face.size();

            m_primitives.clear();
            m_primitives.reserve(numTriangles);

            Primitive q;
            //#pragma omp parallel for private (q)
            for (uint t = 0; t < numTriangles; ++t)
            {
                m_em.generateFacePrimitive(q, m_dcel->m_face[t], *m_dcel, m_mean_edge_size, m_scale);

            //#pragma omp critical
                m_primitives.push_back(q);
            }
        }

        template <class ErrorMetric>
        void ProgressiveMesh<ErrorMetric>::updateFacesQuadrics(Index vsIndex)
        {
            // We go all over the faces which contain vsIndex
            VFIterator vsfIt = VFIterator(m_dcel->m_vertex[vsIndex]);
            FaceList adjFaces = vsfIt.list();
            for (uint t = 0; t < adjFaces.size(); ++t)
            {
                Primitive q;
                m_em.generateFacePrimitive(q, adjFaces[t], *m_dcel, m_mean_edge_size, m_scale);
                m_primitives[adjFaces[t]->idx] = q;
            }
        }

        template <class ErrorMetric>
        Scalar ProgressiveMesh<ErrorMetric>::getWedgeAngle(Index faceIndex, Index vsIndex, Index vtIndex)
        {
            Scalar wedgeAngle;
            Face_ptr face = m_dcel->m_face[faceIndex];
            Vertex_ptr vs = m_dcel->m_vertex[vsIndex];
            Vertex_ptr vt = m_dcel->m_vertex[vtIndex];

            HalfEdge_ptr he = face->HE();
            for (int i = 0; i < 3; i++)
            {
                if (he->V() == vs || he->V() == vt)
                {
                    Vector3 v0 = he->Next()->V()->P() - he->V()->P();
                    Vector3 v1 = he->Prev()->V()->P() - he->V()->P();
                    v0.normalize();
                    v1.normalize();
                    wedgeAngle = std::acos(v0.dot(v1));
                    break;
                }
                he = he->Next();
            }
            CORE_ASSERT(wedgeAngle < 360, "WEDGE ANGLE WAY TOO HIGH");
            return wedgeAngle;
        }

        template<class ErrorMetric>
        typename ErrorMetric::Primitive ProgressiveMesh<ErrorMetric>::computeVertexQuadric(Index vertexIndex)
        {
            VFIterator vIt = VFIterator(m_dcel->m_vertex[vertexIndex]);
            FaceList adjFaces = vIt.list();

            // We go all over the faces which contain vIndex
            // We add the quadrics of all the faces
            Primitive q, qToAdd;
            Index fIdx;

            Scalar weight = 1.0/adjFaces.size();
            q = m_primitives[adjFaces[0]->idx];
            q.applyPrattNorm();

            for (unsigned int i = 1; i < adjFaces.size(); i++)
            {
                Face_ptr f = adjFaces[i];
                fIdx = f->idx;

                qToAdd = m_primitives[fIdx];
                qToAdd.applyPrattNorm();

                Primitive qtest;
                if (i == 1)
                    qtest = m_em.combine(qToAdd, weight, q, weight);
                else
                    qtest = m_em.combine(qToAdd, weight, q, 1);
                q = qtest;
                q.applyPrattNorm();
            }
            return q;
        }

        template <class ErrorMetric>
        typename ErrorMetric::Primitive ProgressiveMesh<ErrorMetric>::computeEdgeQuadric(Index halfEdgeIndex, std::ofstream &file)
        {
            EFIterator eIt = EFIterator(m_dcel->m_halfedge[halfEdgeIndex]);
            FaceList adjFaces = eIt.list();

            // We go all over the faces which contain vs and vt
            // We add the quadrics of all the faces
            Primitive q, qToAdd;
            Index fIdx;

            Scalar weight = 1.0/adjFaces.size();
            q = m_primitives[adjFaces[0]->idx];
            //q *= weight;
            q.applyPrattNorm();

            Vector3 normal0 = Geometry::triangleNormal(adjFaces[0]->HE()->V()->P(), adjFaces[0]->HE()->Next()->V()->P(), adjFaces[0]->HE()->Next()->Next()->V()->P());

            for (unsigned int i = 1; i < adjFaces.size(); i++)
            {
                Face_ptr f = adjFaces[i];
                Vector3 normali = Geometry::triangleNormal(f->HE()->V()->P(), f->HE()->Next()->V()->P(), f->HE()->Next()->Next()->V()->P());
                Scalar test = normal0.dot(normali);
                if (test < -0.9)
                    LOG(logINFO) << "inversion normales";
                normal0 = (normal0+normali)/2.0;

                fIdx = f->idx;
//              Scalar area = Ra::Core::Geometry::triangleArea
//                                ( f->HE()->V()->P(),
//                                  f->HE()->Next()->V()->P(),
//                                  f->HE()->Prev()->V()->P());
                //Scalar wedgeAngle = getWedgeAngle(fIdx,
                //                                m_dcel->m_halfedge[halfEdgeIndex]->V()->idx,
                //                                m_dcel->m_halfedge[halfEdgeIndex]->Next()->V()->idx);
                qToAdd = m_primitives[fIdx];
                //qToAdd *= weight;
                qToAdd.applyPrattNorm();

                //LOG(logINFO) << "q1 : " << q.center().x() << "; " << q.center().y() << "; " << q.center().z() << "/ " << q.radius();
                //LOG(logINFO) << "q1 : " << q.tau() << ", " << q.eta().transpose() << ", " << q.kappa();
                //file << q.m_uc << " " << q.m_ul.transpose() << " " << q.m_uq << " " << q.m_p.transpose() << "\n";
                //file << q.center().x() << " " << q.center().y() << " " << q.center().z() << " " << q.radius() << "\n";


                //LOG(logINFO) << "q2 : " << qToAdd.center().x() << "; " << qToAdd.center().y() << "; " << qToAdd.center().z() << "/ " << qToAdd.radius();
                //LOG(logINFO) << "q2 : " << qToAdd.tau() << ", " << qToAdd.eta().transpose() << ", " << qToAdd.kappa();
                //file << qToAdd.m_uc << " " << qToAdd.m_ul.transpose() << " " << qToAdd.m_uq << " " << qToAdd.m_p.transpose() << "\n";
                //file << qToAdd.center().x() << " " << qToAdd.center().y() << " " << qToAdd.center().z() << " " << qToAdd.radius() << "\n";


                //q = m_em.combine(qToAdd, q);
                Primitive qtest;
                if (i == 1)
                    qtest = m_em.combine(qToAdd, weight, q, weight, file);
                else
                    qtest = m_em.combine(qToAdd, weight, q, 1, file);

                /*
                Scalar test = qtest.m_ul.squaredNorm() - Scalar(4.) * qtest.m_uc * qtest.m_uq;
                if (test < 0)
                {
                    LOG(logINFO) << "combine norm < 0";
                    file << q.m_uc << " " << q.m_ul.transpose() << " " << q.m_uq << " " << q.m_p.transpose() << "\n";
                    file << q.center().x() << " " << q.center().y() << " " << q.center().z() << " " << q.radius() << "\n";
                    file << qToAdd.m_uc << " " << qToAdd.m_ul.transpose() << " " << qToAdd.m_uq << " " << qToAdd.m_p.transpose() << "\n";
                    file << qToAdd.center().x() << " " << qToAdd.center().y() << " " << qToAdd.center().z() << " " << qToAdd.radius() << "\n";
                    file << qtest.m_uc << " " << qtest.m_ul.transpose() << " " << qtest.m_uq << " " << qtest.m_p.transpose() << "\n";
                    file << qtest.center().x() << " " << qtest.center().y() << " " << qtest.center().z() << " " << qtest.radius() << "\n";
                }
                */
                q = qtest;
                q.applyPrattNorm();

            }

            //q.applyPrattNorm();
            return q;
        }

        template <class ErrorMetric>
        typename ErrorMetric::Primitive ProgressiveMesh<ErrorMetric>::computeEdgeQuadric(Index halfEdgeIndex)
        {
            EFIterator eIt = EFIterator(m_dcel->m_halfedge[halfEdgeIndex]);
            FaceList adjFaces = eIt.list();

            // We go all over the faces which contain vs and vt
            // We add the quadrics of all the faces
            Primitive q, qToAdd;
            Index fIdx;

            Scalar weight = 1.0/adjFaces.size();
            q = m_primitives[adjFaces[0]->idx];
            //q *= weight;
            q.applyPrattNorm();

            for (unsigned int i = 1; i < adjFaces.size(); i++)
            {
                Face_ptr f = adjFaces[i];
                fIdx = f->idx;
//              Scalar area = Ra::Core::Geometry::triangleArea
//                                ( f->HE()->V()->P(),
//                                  f->HE()->Next()->V()->P(),
//                                  f->HE()->Prev()->V()->P());
                Scalar wedgeAngle = getWedgeAngle(fIdx,
                                                m_dcel->m_halfedge[halfEdgeIndex]->V()->idx,
                                                m_dcel->m_halfedge[halfEdgeIndex]->Next()->V()->idx);
                qToAdd = m_primitives[fIdx];
                //qToAdd *= weight;
                qToAdd.applyPrattNorm();

                //LOG(logINFO) << "q1 : " << q.center().x() << "; " << q.center().y() << "; " << q.center().z() << "/ " << q.radius();
                //LOG(logINFO) << "q1 : " << q.tau() << ", " << q.eta().transpose() << ", " << q.kappa();
                //LOG(logINFO) << q.m_uc << " " << q.m_ul.transpose() << " " << q.m_uq << " " << q.m_p;

                //LOG(logINFO) << "q2 : " << qToAdd.center().x() << "; " << qToAdd.center().y() << "; " << qToAdd.center().z() << "/ " << qToAdd.radius();
                //LOG(logINFO) << "q2 : " << qToAdd.tau() << ", " << qToAdd.eta().transpose() << ", " << qToAdd.kappa();
                //LOG(logINFO) << qToAdd.m_uc << " " << qToAdd.m_ul.transpose() << " " << qToAdd.m_uq << " " << qToAdd.m_p;

                Primitive qtest;
                if (i == 1)
                    qtest = m_em.combine(qToAdd, weight, q, weight);
                else
                    qtest = m_em.combine(qToAdd, weight, q, 1);
                q.applyPrattNorm();

                //LOG(logINFO) << "r  : " << q.center().x() << "; " << q.center().y() << "; " << q.center().z() << "/ " << q.radius();
                //LOG(logINFO) << "r  : " << q.tau() << ", " << q.eta().transpose() << ", " << q.kappa();
            }
            q.applyPrattNorm();
            return q;
        }

        //-----------------------------------------------------

        template <class ErrorMetric>
        Scalar ProgressiveMesh<ErrorMetric>::computeEdgeError(Index halfEdgeIndex, Vector3 &pResult, Primitive &q, std::ofstream &file)
        {
            q = computeEdgeQuadric(halfEdgeIndex, file);
            Vector3 vs = m_dcel->m_halfedge[halfEdgeIndex]->V()->P();
            Vector3 vt = m_dcel->m_halfedge[halfEdgeIndex]->Next()->V()->P();
            Scalar error = m_em.computeError(q, vs, vt, pResult);
            return error;
        }

        template <class ErrorMetric>
        Scalar ProgressiveMesh<ErrorMetric>::computeEdgeError(Index halfEdgeIndex, Vector3 &pResult, Primitive &q)
        {
            q = computeEdgeQuadric(halfEdgeIndex);
            Vector3 vs = m_dcel->m_halfedge[halfEdgeIndex]->V()->P();
            Vector3 vt = m_dcel->m_halfedge[halfEdgeIndex]->Next()->V()->P();
            Scalar error = m_em.computeError(q, vs, vt, pResult);
            return error;
        }

        //--------------------------------------------------

        template <class ErrorMetric>
        PriorityQueue ProgressiveMesh<ErrorMetric>::constructPriorityQueue()
        {
            PriorityQueue pQueue = PriorityQueue();
            const uint numTriangles = m_dcel->m_face.size();
            //pQueue.reserve(numTriangles*3 / 2);

            // parcours des aretes
            double edgeError;
            Vector3 p = Vector3::Zero();
            int j;

            //std::ofstream myfile;
            //myfile.open ("algebraic_spheres_data.txt");
            m_primitives_he.reserve(numTriangles*3);
            m_primitives_v.reserve(numTriangles*3);
            for (int i = 0; i < numTriangles*3; i++)
            {
                m_primitives_he.push_back(Primitive());
                m_primitives_v.push_back(Primitive());
            }

//#pragma omp parallel for private(j, edgeError, p)
            for (unsigned int i = 0; i < numTriangles; i++)
            {
                const Face_ptr& f = m_dcel->m_face.at( i );
                HalfEdge_ptr h = f->HE();
                for (j = 0; j < 3; j++)
                {
                    const Vertex_ptr& vs = h->V();
                    const Vertex_ptr& vt = h->Next()->V();

                    // To prevent adding twice the same edge
                    if (vs->idx > vt->idx)
                    {
                        h = h->Next();
                        continue;
                    }

                    //edgeError = computeEdgeError(f->HE()->idx, p, myfile);
                    Primitive q;
                    edgeError = computeEdgeError(h->idx, p, q);
                    m_primitives_he[h->idx] = q;

                    m_primitives_v[vs->idx] = computeVertexQuadric(vs->idx);
                    m_primitives_v[vt->idx] = computeVertexQuadric(vt->idx);

//#pragma omp critical
                    {
                        pQueue.insert(PriorityQueue::PriorityQueueData(vs->idx, vt->idx, h->idx, i, edgeError, p));
                    }
                    h = h->Next();
                }
            }

            //myfile.close();

            pQueue.display();


            /* TEST PROJECTION SUR SPHERES */

            // Primitive par arete
            /*
            for (unsigned int i = 0; i < numTriangles; i++)
            {
                const Face_ptr& f = m_dcel->m_face.at( i );
                HalfEdge_ptr h = f->HE();
                for (j = 0; j < 3; j++)
                {
                    const Vertex_ptr& vs = h->V();
                    const Vertex_ptr& vt = h->Next()->V();

                    // To prevent adding twice the same edge
                    if (vs->idx > vt->idx) continue;

                    Primitive qtest = m_primitives_he[h->idx];
                    Vector3 new_vs = qtest.project(vs->P());
                    Vector3 new_vt = qtest.project(vt->P());
                    m_dcel->m_vertex[vs->idx]->setP(new_vs);
                    m_dcel->m_vertex[vt->idx]->setP(new_vt);
                    h = h->Next();
                }
            }
            */

            // primitive par face
            /*
            for (unsigned int i = 0; i < numTriangles; i++)
            {
                const Face_ptr& f = m_dcel->m_face.at( i );
                HalfEdge_ptr h = f->HE();
                for (j = 0; j < 3; j++)
                {
                    const Vertex_ptr& v = h->V();

                    Primitive qtest = m_primitives[f->idx];
                    Vector3 new_v = qtest.project(v->P());
                    m_dcel->m_vertex[v->idx]->setP(new_v);
                    h = h->Next();
                }
            }
            */

            /* FIN TEST PROJECTION SUR SPHERES */

            return pQueue;
        }

        template <class ErrorMetric>
        void ProgressiveMesh<ErrorMetric>::updatePriorityQueue(PriorityQueue &pQueue, Index vsIndex, Index vtIndex, std::ofstream &file)
        {
            // we delete of the priority queue all the edge containing vs_id or vt_id
            pQueue.removeEdges(vsIndex);
            pQueue.removeEdges(vtIndex);

            double edgeError;
            Vector3 p = Vector3::Zero();
            Index vIndex;

            VHEIterator vsHEIt = VHEIterator(m_dcel->m_vertex[vsIndex]);
            HalfEdgeList adjHE = vsHEIt.list();

            for (uint i = 0; i < adjHE.size(); i++)
            {
                HalfEdge_ptr he = adjHE[i];

                Primitive q;
                edgeError = computeEdgeError(he->idx, p, q, file);

                m_primitives_he[he->idx] = q;

                m_primitives_v[he->V()->idx] = computeVertexQuadric(he->V()->idx);
                m_primitives_v[he->Next()->V()->idx] = computeVertexQuadric(he->Next()->V()->idx);


                vIndex = he->Next()->V()->idx;
                pQueue.insert(PriorityQueue::PriorityQueueData(vsIndex, vIndex, he->idx, he->F()->idx, edgeError, p));
            }
            //pQueue.display();
        }

        //--------------------------------------------------

        template <class ErrorMetric>
        bool ProgressiveMesh<ErrorMetric>::isEcolPossible(Index halfEdgeIndex, Vector3 pResult)
        {
            HalfEdge_ptr he = m_dcel->m_halfedge[halfEdgeIndex];

            // Look at configuration T inside a triangle
            bool hasTIntersection = false;
            VVIterator v1vIt = VVIterator(he->V());
            VVIterator v2vIt = VVIterator(he->Next()->V());
            VertexList adjVerticesV1 = v1vIt.list();
            VertexList adjVerticesV2 = v2vIt.list();

            uint countIntersection = 0;
            for (uint i = 0; i < adjVerticesV1.size(); i++)
            {
                for (uint j = 0; j < adjVerticesV2.size(); j++)
                {
                    if (adjVerticesV1[i]->idx == adjVerticesV2[j]->idx)
                        countIntersection++;
                }
            }
            if (countIntersection > 2)
            {
                LOG(logINFO) << "The edge " << he->V()->idx << ", " << he->Next()->V()->idx << " in face " << he->F()->idx << " is not collapsable for now : T-Intersection";
                hasTIntersection = true;
                return false;
            }

            // Look if normals of faces change after collapse

            bool isFlipped = false;
            EFIterator eIt = EFIterator(he);
            FaceList adjFaces = eIt.list();

            Index vsId = he->V()->idx;
            Index vtId = he->Next()->V()->idx;
            for (uint i = 0; i < adjFaces.size(); i++)
            {
                HalfEdge_ptr heCurr = adjFaces[i]->HE();
                Vertex_ptr v1 = nullptr;
                Vertex_ptr v2 = nullptr;
                Vertex_ptr v = nullptr;
                for (uint j = 0; j < 3; j++)
                {
                    if (heCurr->V()->idx != vsId && heCurr->V()->idx != vtId)
                    {
                        if (v1 == nullptr)
                            v1 = heCurr->V();
                        else if (v2 == nullptr)
                            v2 = heCurr->V();
                    }
                    else
                    {
                        v = heCurr->V();
                    }
                    heCurr = heCurr->Next();
                }
                if (v1 != nullptr && v2 != nullptr)
                {
                    Vector3 d1 = v1->P() - pResult;
                    Vector3 d2 = v2->P() - pResult;
                    d1.normalize();
                    d2.normalize();

                    //TEST
                    //Do we really need this ?

                    //Scalar a = fabs(d1.dot(d2));
                    //Vector3 d1_before = v1->P() - v->P();
                    //Vector3 d2_before = v2->P() - v->P();
                    //d1_before.normalize();
                    //d2_before.normalize();
                    //Scalar a_before = fabs(d1_before.dot(d2_before));
                    //if (a > 0.999 && a_before < 0.999)
                    //    isFlipped = true;


                    Vector3 fp_n = d1.cross(d2);
                    fp_n.normalize();
                    Vector3 f_n = Geometry::triangleNormal(v->P(), v1->P(), v2->P());
                    Scalar fpnDotFn = fp_n.dot(f_n);
                    if (fpnDotFn < -0.5)
                    {
                        isFlipped = true;
                        LOG(logINFO) << "The edge " << he->V()->idx << ", " << he->Next()->V()->idx << " in face " << he->F()->idx << " is not collapsable for now : Flipped face";
                        return false;
                        break;
                    }

                }
            }

            return ((!hasTIntersection) && (!isFlipped));


            //return !hasTIntersection;
        }

        //--------------------------------------------------

        template <class ErrorMetric>
        std::vector<ProgressiveMeshData> ProgressiveMesh<ErrorMetric>::constructM0(int targetNbFaces, int &nbNoFrVSplit, bool primitiveUpdate, Scalar scale)
        {
            uint nbPMData = 0;
            m_scale = scale;

            std::vector<ProgressiveMeshData> pmdata;
            pmdata.reserve(targetNbFaces);

            LOG(logINFO) << "Computing Faces Quadrics...";
            computeFacesQuadrics();

            LOG(logINFO) << "Computing Priority Queue...";
            PriorityQueue pQueue = constructPriorityQueue();
            PriorityQueue::PriorityQueueData d;

            LOG(logINFO) << "Collapsing...";
            ProgressiveMeshData data;

            std::ofstream myfile;
            myfile.open ("algebraic_spheres_data.txt");
            while (m_nb_faces > targetNbFaces)
            {
                if (pQueue.empty()) break;
                d = pQueue.top();

                HalfEdge_ptr he = m_dcel->m_halfedge[d.m_edge_id];

                // TODO !
                if (!isEcolPossible(he->idx, d.m_p_result))
                    continue;

                if (he->Twin() == nullptr)
                {
                    m_nb_faces -= 1;
                    nbNoFrVSplit++;
                }
                else
                {
                    m_nb_faces -= 2;
                }
                m_nb_vertices -= 1;

                data = DcelOperations::edgeCollapse(*m_dcel, d.m_edge_id, d.m_p_result);
                updateFacesQuadrics(d.m_vs_id);
                updatePriorityQueue(pQueue, d.m_vs_id, d.m_vt_id, myfile);

                // if debug
                data.setError(d.m_err);
                data.setPResult(d.m_p_result);
                data.setQCenter(m_primitives_he[he->idx].center());
                data.setQRadius(m_primitives_he[he->idx].radius());
                data.setQ1Center(m_primitives_v[d.m_vs_id].center());
                data.setQ1Radius(m_primitives_v[d.m_vs_id].radius());
                data.setQ2Center(m_primitives_v[d.m_vt_id].center());
                data.setQ2Radius(m_primitives_v[d.m_vt_id].radius());
                // end if

                pmdata.push_back(data);

                nbPMData++;
            }
            myfile.close();
            LOG(logINFO) << "Collapsing done";

            return pmdata;
        }

        //--------------------------------------------------

        template <class ErrorMetric>
        void ProgressiveMesh<ErrorMetric>::vsplit(ProgressiveMeshData pmData)
        {
            HalfEdge_ptr he = m_dcel->m_halfedge[pmData.getHeFlId()];
            if (he->Twin() == NULL)
                m_nb_faces += 1;
            else
                m_nb_faces += 2;
            m_nb_vertices += 1;

            //LOG(logINFO) << "Vertex Split " << pmData.getVsId() << ", " << pmData.getVtId() << ", faces " << pmData.getFlId() << ", " << pmData.getFrId();

            DcelOperations::vertexSplit(*m_dcel, pmData);
        }

        template <class ErrorMetric>
        void ProgressiveMesh<ErrorMetric>::ecol(ProgressiveMeshData pmData)
        {
            HalfEdge_ptr he = m_dcel->m_halfedge[pmData.getHeFlId()];
            if (he->Twin() == NULL)
                m_nb_faces -= 1;
            else
                m_nb_faces -= 2;
            m_nb_vertices -= 1;

            //LOG(logINFO) << "Edge Collapse " << pmData.getVsId() << ", " << pmData.getVtId() << ", faces " << pmData.getFlId() << ", " << pmData.getFrId();

            DcelOperations::edgeCollapse(*m_dcel, pmData);
        }
    }
}
