#ifndef MEMORY_POOL_H
#define MEMORY_POOL_H

#include <QObject>
#include <QMutex>
#include <QQueue>
#include <QVector>
#include <QByteArray>
#include <QDebug>

// 图像数据内存池
class ImageMemoryPool : public QObject
{
    Q_OBJECT
    
public:
    struct MemoryBlock {
        void* data;
        size_t size;
        bool inUse;
        qint64 lastUsed;
        
        MemoryBlock() : data(nullptr), size(0), inUse(false), lastUsed(0) {}
        MemoryBlock(void* ptr, size_t sz) : data(ptr), size(sz), inUse(false), lastUsed(0) {}
    };
    
    explicit ImageMemoryPool(QObject *parent = nullptr);
    ~ImageMemoryPool();
    
    // 分配指定大小的内存
    void* allocate(size_t size);
    
    // 释放内存
    void deallocate(void* ptr);
    
    // 预分配常用尺寸的内存块
    void preallocate(const QVector<size_t>& commonSizes);
    
    // 清理长时间未使用的内存块
    void cleanupUnusedBlocks(qint64 timeoutMs = 30000);
    
    // 获取内存池状态
    void getStats(size_t& totalBlocks, size_t& usedBlocks, size_t& totalSize) const;
    
private:
    QVector<MemoryBlock> m_blocks;
    mutable QMutex m_mutex;
    
    // 查找合适的内存块
    int findSuitableBlock(size_t size);
    
    // 创建新的内存块
    void createBlock(size_t size);
};

// 网络缓冲区池
class NetworkBufferPool : public QObject
{
    Q_OBJECT
    
public:
    explicit NetworkBufferPool(size_t bufferSize = 4096, int initialCount = 10, QObject *parent = nullptr);
    ~NetworkBufferPool();
    
    // 获取缓冲区
    QByteArray getBuffer();
    
    // 归还缓冲区
    void returnBuffer(const QByteArray& buffer);
    
    // 调整缓冲区大小
    void resizeBuffers(size_t newSize);
    
    // 获取池状态
    void getStats(int& totalBuffers, int& availableBuffers) const;
    
private:
    QQueue<QByteArray> m_bufferPool;
    size_t m_bufferSize;
    mutable QMutex m_mutex;
    
    // 创建新的缓冲区
    void createBuffers(int count);
};

// 全局内存池管理器
class MemoryPoolManager : public QObject
{
    Q_OBJECT
    
public:
    static MemoryPoolManager& instance();
    
    ImageMemoryPool* imagePool() { return m_imagePool; }
    NetworkBufferPool* networkPool() { return m_networkPool; }
    
    // 初始化内存池
    void initialize();
    
    // 清理所有内存池
    void cleanup();
    
private:
    explicit MemoryPoolManager(QObject *parent = nullptr);
    ~MemoryPoolManager();
    
    ImageMemoryPool* m_imagePool;
    NetworkBufferPool* m_networkPool;
    
    static MemoryPoolManager* s_instance;
};

// 便捷宏定义
#define GET_IMAGE_POOL() MemoryPoolManager::instance().imagePool()
#define GET_NETWORK_POOL() MemoryPoolManager::instance().networkPool()

#endif // MEMORY_POOL_H 