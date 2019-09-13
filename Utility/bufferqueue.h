#ifndef BUFFERQUEUE_H
#define BUFFERQUEUE_H

#include <QDebug>
#include <QSemaphore>

template <class T> class BufferQueue
{
public:
    BufferQueue(int bufferSize = 100) {
        setBufferSize(bufferSize);
    }

    ~BufferQueue() {
        init();
        QVector<T>().swap(m_bufferQueue);
    }

    void setBufferSize(int bufferSize) {
        m_bufferSize = bufferSize;
        m_bufferQueue = QVector<T>(bufferSize);
        m_useableSpace.reset(new QSemaphore(0));
        m_freeSpace.reset(new QSemaphore(m_bufferSize));
        m_front = m_rear = 0;
    }

    void enqueue(const T &element) {
#ifndef QT_NO_DEBUG_OUTPUT
        qDebug() << "[freespace " << m_freeSpace->available()
                 << "] --- [useable " << m_useableSpace->available() << "]";
#endif
        m_freeSpace->acquire();
        m_bufferQueue[m_front++ % m_bufferSize] = element;
        QSemaphoreReleaser releaser(m_useableSpace.get());
    }

    T dequeue() {
#ifndef QT_NO_DEBUG_OUTPUT
        qDebug() << "[freespace " << m_freeSpace->available()
                 << "] --- [useable " << m_useableSpace->available() << "]";
#endif
        m_useableSpace->acquire();
        T element = m_bufferQueue[m_rear++ % m_bufferSize];
        m_freeSpace->release();

        return element;
    }

    void init() {
        m_useableSpace->acquire(m_useableSpace->available());
        m_freeSpace->release(m_bufferSize - m_freeSpace->available());
        m_front.store(0);
        m_rear.store(0);
    }

private:
    //         -1               +1
    //   [free space] -> [useable space]
    QScopedPointer<QSemaphore> m_freeSpace;
    QScopedPointer<QSemaphore> m_useableSpace;
    std::atomic_int m_rear{0};
    std::atomic_int m_front{0};
    QVector<T> m_bufferQueue;
    int m_bufferSize;

    //我讨厌warning
    char paddingByte[4];
};

#endif
