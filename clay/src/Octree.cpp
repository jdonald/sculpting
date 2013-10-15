#include "Octree.h"

/** Constructor */
Octree::Octree(Octree* parent, int depth) : parent_(parent), aabbLoose_(), aabbSplit_(), iTris_(), depth_(depth)
{
    for (int i=0;i<8;++i)
        child_[i]=0;
}

/** Destructor */
Octree::~Octree()
{
    parent_ = 0;
    if(child_[0]!=0)
    {
        for (int i=0;i<8;++i)
        {
            delete child_[i];
            child_[i] = 0;
        }
    }
}

/** Return triangles (in case of a leaf) */
std::vector<int> &Octree::getTriangles() { return iTris_; }
Aabb &Octree::getAabbLoose() { return aabbLoose_; }
Aabb &Octree::getAabbSplit() { return aabbSplit_; }
int Octree::getDepth() const { return depth_; }
Octree* Octree::getParent() { return parent_; }
Octree** Octree::getChildren() { return child_; }

/** Build octree */
void Octree::build(Mesh *mesh, const std::vector<int> &iTris, const Aabb &aabb)
{
    aabbSplit_ = aabb;
    aabbLoose_ = aabb;
    iTris_.clear();
    TriangleVector &triangles = mesh->getTriangles();
    int nbTriangles=iTris.size();
    if(parent_ && parent_->child_[7]==this)
        for(int i=0;i<nbTriangles;++i)
        {
            Triangle &t = triangles[iTris[i]];
            if(t.tagFlag_!=Triangle::tagMask_)
            {
                aabbLoose_.expand(t.aabb_);
                iTris_.push_back(iTris[i]);
            }
        }
    else
        for(int i=0;i<nbTriangles;++i)
        {
            Triangle &t = triangles[iTris[i]];
            if(aabbSplit_.pointInside(t.aabb_.getCenter()) && t.tagFlag_!=Triangle::tagMask_)
            {
                aabbLoose_.expand(t.aabb_);
                iTris_.push_back(iTris[i]);
            }
        }
    int nbTrianglesCell=iTris_.size();
    if (nbTrianglesCell > Octree::maxTriangles_ && depth_ < Octree::maxDepth_)
        constructCells(mesh);
    else
    {
        for(int i=0;i<nbTrianglesCell;++i)
        {
            Triangle &t = triangles[iTris_[i]];
            t.tagFlag_ = Triangle::tagMask_;
            t.leaf_ = this;
            t.posInLeaf_ = i;
        }
    }
}

/** Construct cell */
void Octree::constructCells(Mesh *mesh)
{
    const Vector3& min = aabbSplit_.min_;
    const Vector3& max = aabbSplit_.max_;
    Vector3 center = (max+min)/2;
    float deltaX = (max.x()-min.x())/2;
    float deltaY = (max.y()-min.y())/2;
    float deltaZ = (max.z()-min.z())/2;
    int nextDepth = depth_+1;
    child_[0] = new Octree(this,nextDepth);
    child_[0]->build(mesh, iTris_, Aabb(min, center));
    child_[1] = new Octree(this,nextDepth);
    child_[1]->build(mesh, iTris_, Aabb(Vector3(min.x()+deltaX,min.y(),min.z()),Vector3(center.x()+deltaX,center.y(),center.z())));
    child_[2] = new Octree(this,nextDepth);
    child_[2]->build(mesh, iTris_, Aabb(Vector3(center.x(),center.y()-deltaY,center.z()),Vector3(max.x(),max.y()-deltaY,max.z())));
    child_[3] = new Octree(this,nextDepth);
    child_[3]->build(mesh, iTris_, Aabb(Vector3(min.x(),min.y(),min.z()+deltaZ),Vector3(center.x(),center.y(),center.z()+deltaZ)));
    child_[4] = new Octree(this,nextDepth);
    child_[4]->build(mesh, iTris_, Aabb(Vector3(min.x(),min.y()+deltaY,min.z()),Vector3(center.x(),center.y()+deltaY,center.z())));
    child_[5] = new Octree(this,nextDepth);
    child_[5]->build(mesh, iTris_, Aabb(Vector3(center.x(),center.y(),center.z()-deltaZ),Vector3(max.x(),max.y(),max.z()-deltaZ)));
    child_[6] = new Octree(this,nextDepth);
    child_[6]->build(mesh, iTris_, Aabb(center,max));
    child_[7] = new Octree(this,nextDepth);
    child_[7]->build(mesh, iTris_, Aabb(Vector3(center.x()-deltaX,center.y(),center.z()),Vector3(max.x()-deltaX,max.y(),max.z())));
    iTris_.clear();
}

/** Draw the octree */
void Octree::draw() const
{
    aabbSplit_.draw(QColor(0,255,0));
    aabbLoose_.draw(QColor(255,0,0));
    if (child_[0]!=0)
        for (int i=0;i<8;++i)
            child_[i]->draw();
}

/** Return triangles in cells hit by a ray */
std::vector<int> Octree::intersectRay(const Vector3& vert, const Vector3& dir) const
{
    if(aabbLoose_.intersectRay(vert,dir))
    {
        if (child_[0]!=0)
        {
            std::vector<int> iTriangles;
            for (int i=0;i<8;++i)
            {
                std::vector<int> iTris = child_[i]->intersectRay(vert, dir);
                iTriangles.insert(iTriangles.end(), iTris.begin(), iTris.end());
            }
            return iTriangles;
        }
        else
            return iTris_;
    }
    return std::vector<int>();
}

/** Return triangles inside a sphere */
std::vector<int> Octree::intersectSphere(const Vector3& vert, float radiusSquared,
                                         std::vector<Octree*> &leavesHit)
{
    if(aabbSplit_.intersectSphere(vert,radiusSquared))
    {
        if (child_[0]!=0)
        {
            std::vector<int> iTriangles;
            for (int i=0;i<8;++i)
            {
                std::vector<int> iTris = child_[i]->intersectSphere(vert, radiusSquared,leavesHit);
                iTriangles.insert(iTriangles.end(), iTris.begin(), iTris.end());
            }
            return iTriangles;
        }
        else
        {
            leavesHit.push_back(this);
            return iTris_;
        }
    }
    return std::vector<int>();
}

/** Add triangle in the octree, subdivide the cell if necessary */
void Octree::addTriangle(Mesh *mesh, Triangle &tri)
{
    if (aabbSplit_.pointInside(tri.aabb_.getCenter()))
    {
        aabbLoose_.expand(tri.aabb_);
        if (child_[0]!=0)
        {
            for (int i=0;i<8;++i)
                child_[i]->addTriangle(mesh,tri);
        }
        else
        {
            tri.posInLeaf_ = iTris_.size();
            tri.leaf_ = this;
            iTris_.push_back(tri.id_);
        }
    }
}

/** Cut leaves if needed */
void Octree::checkEmptiness(Octree* leaf, std::vector<Octree*> &cutLeaves)
{
    Octree* parent = leaf->getParent();
    if(parent)
    {
        Octree **child=parent->getChildren();
        for(int i=0;i<8;++i)
            if(child[i]->getTriangles().size() || child[i]->getChildren()[0]!=0)
                return;
        cutLeaves.push_back(parent);
        for(int i=0;i<8;++i)
            cutLeaves.push_back(child[i]);
        Octree::checkEmptiness(parent,cutLeaves);
    }
}
