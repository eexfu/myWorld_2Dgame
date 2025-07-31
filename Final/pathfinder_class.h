#ifndef PATHFINDER_H
#define PATHFINDER_H
#include <vector>
#include <functional>
#include <queue>
#include <iostream>
#include <limits>
#include <cmath>
#include <memory>
#include <chrono>
#include <qloggingcategory.h>

/*
 * A_star has 2 template parameters, T and U
 * T is the Node object which needs to have the fields as defined by the concept validNodeType
 *    3 floats: f, g and h to store the cost parameters of the A* algorithm
 *    a boolean visited which is set when you have already been to this node (~ closed list)
 *    a pointer to a Node: prev: the nodefrom which you got to this position
 *    Node should derive from Tile, so you can call getValue()
 * U is a position, passed a pointer, something which derives from Tile (or a Tile)
 * The comparator is used by the priority_queue to put the element with the lowest total cost in front
 * since nodes is a 1D vector representing a 2D grid, we need know the width of this grid
 * heurWeight is the heuristic weight paramters defined by the A* algorithm
 *
 * The function returns a list of moves (0 to 7) using following encoding. The central point is your current position
 *
 *         7 0 1
 *         6   2
 *         5 4 3
 */

template <typename T>
using Comparator = std::function<bool(const T&, const T&)>;

template <typename T>
using pq = std::priority_queue<T, std::vector<T>, Comparator<T>>;

template<typename U>
concept validPosType = requires(U * u)

{
    {u->getXPos()} -> std::same_as<int>;
    {u->getYPos()} -> std::same_as<int>;
};

template<typename T>
concept validNodeType = validPosType<T> && requires(T t)
{
    {T::f} -> std::same_as<float&>;
    {T::g} -> std::same_as<float&>;
    {T::h} -> std::same_as<float&>;
    {T::visited} -> std::same_as<bool&>;
    {T::closed} -> std::same_as<bool&>;
    {T::prev} -> std::same_as<T*&>;
    {t.getValue()} -> std::same_as<float>;
};

template <validPosType U> //helper_func can be used to both calculate cost and heuristic cost in the world
using helper_func = std::function<float(const U & , const U &)>;

template <validNodeType T, validPosType U>
class PathFinder
{
private:
  std::vector<T> & nodes;
    const U * start;
    const U* destination;

    unsigned int width;
    unsigned int height;

    pq<T> openList;
    Comparator<T> & comp;
    helper_func<U> costf;
    helper_func<U> distf;

    float heurWeight;

    QLoggingCategory pathfinderCategory{"pathfinder", QtCriticalMsg};
    float sqrt2;

    //debug variables
    unsigned int nodesChecked = 0;
    unsigned int nodesAddedFirstTime = 0;
    unsigned int nodesReparented = 0;
    unsigned int nodesReparentRejected = 0;
    unsigned int nodesOutOfBounds = 0;
    unsigned int nodesWalls = 0;
    unsigned int nodesReparentedWhenClosed = 0;
    unsigned int nodesReparentRejectedWhenClosed = 0;

public:
    PathFinder(std::vector<T> & N, const U * s, const U * d, Comparator<T>& c, unsigned int w, helper_func<U> cost, helper_func<U> dist, float hW) :
      nodes{N}, start{s}, destination{d}, width{w}, comp{c}, costf{cost}, distf{dist}, heurWeight{hW}
    {
        height = nodes.size() / width;
        sqrt2 = std::sqrt(2);
    };

    std::vector<int> A_star()
    {
        auto startTime = std::chrono::high_resolution_clock::now();

        openList = pq<T>(comp);
        std::vector<int> path;
        T* currentPathNode = &(nodes.at(start->getYPos()*width+start->getXPos()));
        currentPathNode->visited = true; // avoid to come here again
        int currentX = 0;
        int currentY = 0;

        while (currentPathNode->getXPos() != destination->getXPos()
               || currentPathNode->getYPos() != destination->getYPos())
        {
            currentX = currentPathNode->getXPos();
            currentY = currentPathNode->getYPos();
            qCDebug(pathfinderCategory) << "Node to continue... (" << currentX << "," << currentY << ")";

            //process neighbors
            checkNeighbour(currentX-1, currentY-1, currentPathNode);
            checkNeighbour(currentX, currentY-1, currentPathNode);
            checkNeighbour(currentX+1, currentY-1, currentPathNode);
            checkNeighbour(currentX-1, currentY, currentPathNode);
            checkNeighbour(currentX+1, currentY, currentPathNode);
            checkNeighbour(currentX-1, currentY+1, currentPathNode);
            checkNeighbour(currentX, currentY+1, currentPathNode);
            checkNeighbour(currentX+1, currentY+1, currentPathNode);

            //this means no path is possible, so we return an empty vector
            if (openList.empty())
            {
                qCDebug(pathfinderCategory) << "Unable to find path";
                return std::vector<int>();
            }
            //update next node to process
            else
            {
                currentPathNode = &(nodes.at(openList.top().getYPos()*width+openList.top().getXPos()));
                currentPathNode->closed = true;
                openList.pop();
                qCDebug(pathfinderCategory) << "currentPathNode from openlist: (" << currentPathNode->getXPos() << "," << currentPathNode->getYPos() << ") --> f:" << currentPathNode->f << ", g:" << currentPathNode->g << ", h:" << currentPathNode->h;
                qCDebug(pathfinderCategory) << "Next node on top of openlist: (" << openList.top().getXPos() << "," << openList.top().getYPos() << ") --> f:" << openList.top().f << ", g:" << openList.top().g << ", h:" << openList.top().h;
                qCDebug(pathfinderCategory) << "Size of openlist: " << openList.size();
            }
        }

        auto endTime = std::chrono::high_resolution_clock::now();
        qDebug() << "path found after" << std::chrono::duration_cast<std::chrono::milliseconds>(endTime-startTime).count() << "mS";
        startTime = endTime;
        qDebug() << "first time:" << nodesAddedFirstTime << "reparented:" <<  nodesReparented << "reparent rejected:" << nodesReparentRejected << "out of bounds:" << nodesOutOfBounds << "walls:" << nodesWalls;
        qDebug() << "total:" << nodesChecked << "sum of all:" << nodesAddedFirstTime + nodesReparented + nodesReparentRejected + nodesOutOfBounds + nodesWalls;
        qDebug() << "reparented when closed:" << nodesReparentedWhenClosed << "reparent rejected when closed:" << nodesReparentRejectedWhenClosed;

        //return list of path to reach destination
        T * dest = currentPathNode;
        //std::cout << "Getting list of moves to take." <<std::endl;
        while (dest->prev != nullptr)
        {
            int deltaX = dest->prev->getXPos() - dest->getXPos();
            int deltaY = dest->prev->getYPos() - dest->getYPos();
            int newMove = deltaX+10*deltaY;
            qCDebug(pathfinderCategory) << "X:" << deltaX << ", Y:" << deltaY;
            switch (newMove)
            {
            case -10: path.push_back(4); break;
            case -9: path.push_back(5); break;
            case 1: path.push_back(6); break;
            case 11: path.push_back(7); break;
            case 10: path.push_back(0); break;
            case 9: path.push_back(1); break;
            case -1: path.push_back(2); break;
            case -11: path.push_back(3); break;
            default: std::cerr << "wrong value for deltaX" << deltaX;
            }
            dest = dest->prev;
        }
        std::reverse(path.begin(), path.end());

        endTime = std::chrono::high_resolution_clock::now();
        qDebug() << "path constructed after" << std::chrono::duration_cast<std::chrono::milliseconds>(endTime-startTime).count() << "mS";

        return path;
    }

    void setStart(const U *newStart){start = newStart;};
    void setDestination(const U *newDest){destination = newDest;};
    void setHeurWeight(float newHW){heurWeight = newHW;};
    void setNodes(std::vector<T> & newNodes){nodes = newNodes;};

  private:
    void checkNeighbour(int nX, int nY, T* parent)
    {
        qCDebug(pathfinderCategory) << "Checking neighbour : (" << nX << "," << nY << ")";
        ++nodesChecked;

        if (nX >= 0 && nX < width && nY >= 0 && nY < height) //out of bounds
        {
            T & nextNode = nodes.at(nY*width+nX);

            if ( nextNode.getValue()!=std::numeric_limits<float>::infinity() && !nextNode.closed ) //wall
            {
                qCDebug(pathfinderCategory) << "Node present at (" << nX << "," << nY << "), val =" << nextNode.getValue();

                float parentG = parent->g;
                float traverseCost = costf(*parent, nextNode);
                if( nX != parent->getXPos() && nY != parent->getYPos() )
                {
                    traverseCost *= sqrt2;
                }

                if (!nextNode.visited) //first time accessing this node
                {
                    nextNode.g = parentG + traverseCost;
                    nextNode.h = distf(nextNode, *destination);
                    nextNode.f = nextNode.g + heurWeight * nextNode.h;
                    nextNode.prev = parent;
                    nextNode.visited = true;

                    qCDebug(pathfinderCategory) << "New node! parent g =" << parentG << ", traverse cost =" << traverseCost << ", h =" << nextNode.h
                                                << "--> new f =" << nextNode.f;
                    qCDebug(pathfinderCategory)<< "Pushing in openlist... (" << nX << "," << nY << ") --> f =" << nextNode.f;
                    openList.push(nextNode);
                    ++nodesAddedFirstTime;
                }
                else //node already visited
                {
                    //check if current path is better than previous
                    qCDebug(pathfinderCategory) << "Revisiting node! parent g =" << parentG << ", traverse cost =" << traverseCost << ", h =" << nextNode.h
                                                << "--> new f =" << parentG + traverseCost + heurWeight * nextNode.h;
                    qCDebug(pathfinderCategory) << "Check if this path is better, cur g =" << nextNode.g << ", new g =" << parentG + traverseCost;
                    if (parentG + traverseCost < nextNode.g)
                    {
                        qCDebug(pathfinderCategory) << "This path is better, reparenting.";
                        nextNode.g = parentG + traverseCost;
                        nextNode.f = nextNode.g + heurWeight * nextNode.h;
                        nextNode.prev = parent;
                        openList.push(nextNode);

                        ++nodesReparented;
                        if( nextNode.closed )
                        {
                            ++nodesReparentedWhenClosed;
                        }
                    }
                    else
                    {
                        qCDebug(pathfinderCategory) << "not better, ignore";
                        ++nodesReparentRejected;
                        if( nextNode.closed )
                        {
                            ++nodesReparentRejectedWhenClosed;
                        }
                    }
                }
            }
            else
            {
                ++nodesWalls;
                qCDebug(pathfinderCategory) << "Wall at: (" << nX << "," << nY << ")";
            }
        }
        else
        {
            ++nodesOutOfBounds;
            qCDebug(pathfinderCategory) << "Out of bounds: (" << nX << "," << nY << ")";
        }
    }
};

#endif // PATHFINDER_H
