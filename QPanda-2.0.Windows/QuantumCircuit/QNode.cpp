#include "QNode.h"
#include "QPanda/QPandaException.h"

QNodeMap _G_QNodeMap;
QNodeMap::QNodeMap() :m_sCount(0)
{

}

QNodeMap::~QNodeMap()
{
    for (auto aiter = m_pQNodeVector.begin(); aiter != m_pQNodeVector.end(); aiter++)
    {
        QNode * pNode = aiter->second.m_pNode;
        //std::cout<<"position = " << pNode->getPosition() << endl;
        //cout << "nodetype ="<< pNode->getNodeType() << endl;
        delete (pNode);
    }
}

QMAP_SIZE QNodeMap::pushBackNode(QNode * pNode)
{
    WriteLock wl(m_sm);
    MapNode temp = { 0, pNode };
    m_sCount++;
    auto a =m_pQNodeVector.insert(pair<QMAP_SIZE, MapNode>(m_sCount,temp));
    return m_sCount;
}


QNode * QNodeMap::getNode(QMAP_SIZE iNum)
{
    ReadLock rl(m_sm);
    if (iNum == -1)
    {
        return nullptr;
    }
    auto aiter = m_pQNodeVector.find(iNum);
    if (m_pQNodeVector.end() == aiter)
        return nullptr;
    return aiter->second.m_pNode;
}

bool QNodeMap::addNodeRefer(QMAP_SIZE sNum)
{
    WriteLock wl(m_sm);
    auto aiter = m_pQNodeVector.find(sNum);
    if (m_pQNodeVector.end() == aiter)
        return false;
    aiter->second.m_iReference++;
    return true;
}


bool QNodeMap::deleteNode(QMAP_SIZE sNum)
{

    ReadLock * rl = new ReadLock(m_sm);
    WriteLock * wl = nullptr;
    auto aiter = m_pQNodeVector.find(sNum);
    if (m_pQNodeVector.end() == aiter)
    {
        delete rl;
        return false;
    }
        
    if (aiter->second.m_iReference > 1)
    {
        delete rl;
        wl = new WriteLock(m_sm);
        aiter->second.m_iReference--;
        delete wl;
    }
    else
    {
        delete rl;
        delete aiter->second.m_pNode;
        WriteLock wl(m_sm);
        m_pQNodeVector.erase(aiter);
    }
    return true;
}

map<QMAP_SIZE, MapNode>::iterator QNodeMap::getEnd()
{
    return  m_pQNodeVector.end();
}



OriginItem::OriginItem() :m_iNodeNum(-1), m_pNext(nullptr), m_pPre(nullptr)
{

}

OriginItem::~OriginItem()
{
    _G_QNodeMap.deleteNode(m_iNodeNum);

}

Item * OriginItem::getNext()const
{
    return m_pNext;
}
Item * OriginItem::getPre()const
{
    return m_pPre;
}
QNode *OriginItem::getNode() const
{
    auto aiter = _G_QNodeMap.getNode(m_iNodeNum);
    return aiter;
}
void  OriginItem::setNext(Item * pItem)
{
    m_pNext = pItem;
}
void OriginItem::setPre(Item * pItem)
{
    m_pPre = pItem;
}
void OriginItem::setNode(QNode * pNode)
{
    m_iNodeNum = pNode->getPosition();
    if (!_G_QNodeMap.addNodeRefer(m_iNodeNum))
        throw exception();
}