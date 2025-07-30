#include "memory_pool.h"
#include <QDateTime>
#include <QTimer>

// ImageMemoryPool 实现
ImageMemoryPool::ImageMemoryPool(QObject *parent)
    : QObject(parent)
{
    // 预分配常用尺寸
    QVector<size_t> commonSizes = {
        1920 * 1080 * 2,    // 1080p 16bit
        1280 * 720 * 2,     // 720p 16bit
        640 * 480 * 2,      // VGA 16bit
        4096,               // 网络缓冲区
        8192,               // 大缓冲区
        16384               // 图像帧缓冲区
    };
    preallocate(commonSizes);
    
    // 定期清理未使用的内存块
    QTimer* cleanupTimer = new QTimer(this);
    connect(cleanupTimer, &QTimer::timeout, this, [this]() {
        cleanupUnusedBlocks(60000); // 1分钟清理一次
    });
    cleanupTimer->start(60000);
}

ImageMemoryPool::~ImageMemoryPool()
{
    QMutexLocker locker(&m_mutex);
    for (auto& block : m_blocks) {
        if (block.data) {
            free(block.data);
            block.data = nullptr;
        }
    }
    m_blocks.clear();
}

void* ImageMemoryPool::allocate(size_t size)
{
    QMutexLocker locker(&m_mutex);
    
    // 查找合适的内存块
    int index = findSuitableBlock(size);
    if (index >= 0) {
        m_blocks[index].inUse = true;
        m_blocks[index].lastUsed = QDateTime::currentMSecsSinceEpoch();
        return m_blocks[index].data;
    }
    
    // 没有找到合适的块，创建新的
    createBlock(size);
    int newIndex = m_blocks.size() - 1;
    m_blocks[newIndex].inUse = true;
    m_blocks[newIndex].lastUsed = QDateTime::currentMSecsSinceEpoch();
    
    return m_blocks[newIndex].data;
}

void ImageMemoryPool::deallocate(void* ptr)
{
    QMutexLocker locker(&m_mutex);
    
    for (auto& block : m_blocks) {
        if (block.data == ptr) {
            block.inUse = false;
            block.lastUsed = QDateTime::currentMSecsSinceEpoch();
            return;
        }
    }
    
    qWarning() << "ImageMemoryPool: 尝试释放未分配的内存指针:" << ptr;
}

void ImageMemoryPool::preallocate(const QVector<size_t>& commonSizes)
{
    QMutexLocker locker(&m_mutex);
    
    for (size_t size : commonSizes) {
        createBlock(size);
    }
    
    qDebug() << "ImageMemoryPool: 预分配了" << commonSizes.size() << "个内存块";
}

void ImageMemoryPool::cleanupUnusedBlocks(qint64 timeoutMs)
{
    QMutexLocker locker(&m_mutex);
    qint64 currentTime = QDateTime::currentMSecsSinceEpoch();
    
    for (int i = m_blocks.size() - 1; i >= 0; --i) {
        if (!m_blocks[i].inUse && 
            (currentTime - m_blocks[i].lastUsed) > timeoutMs) {
            
            free(m_blocks[i].data);
            m_blocks.removeAt(i);
        }
    }
}

void ImageMemoryPool::getStats(size_t& totalBlocks, size_t& usedBlocks, size_t& totalSize) const
{
    QMutexLocker locker(&m_mutex);
    
    totalBlocks = m_blocks.size();
    usedBlocks = 0;
    totalSize = 0;
    
    for (const auto& block : m_blocks) {
        if (block.inUse) {
            usedBlocks++;
        }
        totalSize += block.size;
    }
}

int ImageMemoryPool::findSuitableBlock(size_t size)
{
    for (int i = 0; i < m_blocks.size(); ++i) {
        if (!m_blocks[i].inUse && m_blocks[i].size >= size) {
            return i;
        }
    }
    return -1;
}

void ImageMemoryPool::createBlock(size_t size)
{
    void* data = malloc(size);
    if (data) {
        m_blocks.append(MemoryBlock(data, size));
    } else {
        qWarning() << "ImageMemoryPool: 内存分配失败，大小:" << size;
    }
}

// NetworkBufferPool 实现
NetworkBufferPool::NetworkBufferPool(size_t bufferSize, int initialCount, QObject *parent)
    : QObject(parent)
    , m_bufferSize(bufferSize)
{
    createBuffers(initialCount);
}

NetworkBufferPool::~NetworkBufferPool()
{
    QMutexLocker locker(&m_mutex);
    m_bufferPool.clear();
}

QByteArray NetworkBufferPool::getBuffer()
{
    QMutexLocker locker(&m_mutex);
    
    if (m_bufferPool.isEmpty()) {
        // 池为空时创建新的缓冲区
        createBuffers(5);
    }
    
    return m_bufferPool.dequeue();
}

void NetworkBufferPool::returnBuffer(const QByteArray& buffer)
{
    QMutexLocker locker(&m_mutex);
    
    // 重置缓冲区大小并清空内容
    QByteArray cleanBuffer;
    cleanBuffer.resize(m_bufferSize);
    cleanBuffer.fill(0);
    
    m_bufferPool.enqueue(cleanBuffer);
}

void NetworkBufferPool::resizeBuffers(size_t newSize)
{
    QMutexLocker locker(&m_mutex);
    
    m_bufferSize = newSize;
    m_bufferPool.clear();
    createBuffers(10);
}

void NetworkBufferPool::getStats(int& totalBuffers, int& availableBuffers) const
{
    QMutexLocker locker(&m_mutex);
    
    totalBuffers = m_bufferPool.size();
    availableBuffers = m_bufferPool.size();
}

void NetworkBufferPool::createBuffers(int count)
{
    for (int i = 0; i < count; ++i) {
        QByteArray buffer;
        buffer.resize(m_bufferSize);
        buffer.fill(0);
        m_bufferPool.enqueue(buffer);
    }
}

// MemoryPoolManager 实现
MemoryPoolManager* MemoryPoolManager::s_instance = nullptr;

MemoryPoolManager& MemoryPoolManager::instance()
{
    if (!s_instance) {
        s_instance = new MemoryPoolManager();
    }
    return *s_instance;
}

MemoryPoolManager::MemoryPoolManager(QObject *parent)
    : QObject(parent)
    , m_imagePool(nullptr)
    , m_networkPool(nullptr)
{
}

MemoryPoolManager::~MemoryPoolManager()
{
    cleanup();
}

void MemoryPoolManager::initialize()
{
    if (!m_imagePool) {
        m_imagePool = new ImageMemoryPool(this);
    }
    if (!m_networkPool) {
        m_networkPool = new NetworkBufferPool(4096, 20, this);
    }
    
    qDebug() << "MemoryPoolManager: 内存池初始化完成";
}

void MemoryPoolManager::cleanup()
{
    if (m_imagePool) {
        delete m_imagePool;
        m_imagePool = nullptr;
    }
    if (m_networkPool) {
        delete m_networkPool;
        m_networkPool = nullptr;
    }
    
    qDebug() << "MemoryPoolManager: 内存池清理完成";
} 