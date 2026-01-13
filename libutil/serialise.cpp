#include "nix/util/serialise.hh"
#include "nix/util/util.hh"

#include <cstring>
#include <cerrno>
#include <memory>

#include <boost/coroutine2/coroutine.hpp>

#  include <poll.h>

namespace nix {

void BufferedSink::operator()(std::string_view data)
{
    if (!buffer)
        buffer = decltype(buffer)(new char[bufSize]);

    while (!data.empty()) {
        /* Optimisation: bypass the buffer if the data exceeds the
           buffer size. */
        if (bufPos + data.size() >= bufSize) {
            flush();
            writeUnbuffered(data);
            break;
        }
        /* Otherwise, copy the bytes to the buffer.  Flush the buffer
           when it's full. */
        size_t n = bufPos + data.size() > bufSize ? bufSize - bufPos : data.size();
        memcpy(buffer.get() + bufPos, data.data(), n);
        data.remove_prefix(n);
        bufPos += n;
        if (bufPos == bufSize)
            flush();
    }
}

void BufferedSink::flush()
{
    if (bufPos == 0)
        return;
    size_t n = bufPos;
    bufPos = 0; // don't trigger the assert() in ~BufferedSink()
    writeUnbuffered({buffer.get(), n});
}

FdSink::~FdSink()
{
    try {
        flush();
    } catch (...) {
        ignoreExceptionInDestructor();
    }
}

void FdSink::writeUnbuffered(std::string_view data)
{
    written += data.size();
    try {
        writeFull(fd, data);
    } catch (SystemError & e) {
        _good = false;
        throw;
    }
}

bool FdSink::good()
{
    return _good;
}

void Source::operator()(char * data, size_t len)
{
    while (len) {
        size_t n = read(data, len);
        data += n;
        len -= n;
    }
}

void Source::operator()(std::string_view data)
{
    (*this)((char *) data.data(), data.size());
}

void Source::drainInto(Sink & sink)
{
    std::array<char, 8192> buf;
    while (true) {
        size_t n;
        try {
            n = read(buf.data(), buf.size());
            sink({buf.data(), n});
        } catch (EndOfFile &) {
            break;
        }
    }
}

std::string Source::drain()
{
    StringSink s;
    drainInto(s);
    return std::move(s.s);
}

size_t BufferedSource::read(char * data, size_t len)
{
    if (!buffer)
        buffer = decltype(buffer)(new char[bufSize]);

    if (!bufPosIn)
        bufPosIn = readUnbuffered(buffer.get(), bufSize);

    /* Copy out the data in the buffer. */
    size_t n = len > bufPosIn - bufPosOut ? bufPosIn - bufPosOut : len;
    memcpy(data, buffer.get() + bufPosOut, n);
    bufPosOut += n;
    if (bufPosIn == bufPosOut)
        bufPosIn = bufPosOut = 0;
    return n;
}

bool BufferedSource::hasData()
{
    return bufPosOut < bufPosIn;
}

size_t FdSource::readUnbuffered(char * data, size_t len)
{
    ssize_t n;
    do {
        n = ::read(fd, data, len);
    } while (n == -1 && errno == EINTR);
    if (n == -1) {
        _good = false;
        throw SysError("reading from file");
    }
    if (n == 0) {
        _good = false;
        throw EndOfFile(std::string(*endOfFileError));
    }
    read += n;
    return n;
}

bool FdSource::good()
{
    return _good;
}

bool FdSource::hasData()
{
    if (BufferedSource::hasData())
        return true;

    while (true) {
        fd_set fds;
        FD_ZERO(&fds);
        int fd_ = fromDescriptorReadOnly(fd);
        FD_SET(fd_, &fds);

        struct timeval timeout;
        timeout.tv_sec = 0;
        timeout.tv_usec = 0;

        auto n = select(fd_ + 1, &fds, nullptr, nullptr, &timeout);
        if (n < 0) {
            if (errno == EINTR)
                continue;
            throw SysError("polling file descriptor");
        }
        return FD_ISSET(fd, &fds);
    }
}

size_t StringSource::read(char * data, size_t len)
{
    if (pos == s.size())
        throw EndOfFile("end of string reached");
    size_t n = s.copy(data, len, pos);
    pos += n;
    return n;
}

void writePadding(size_t len, Sink & sink)
{
    if (len % 8) {
        char zero[8];
        memset(zero, 0, sizeof(zero));
        sink({zero, 8 - (len % 8)});
    }
}

void writeString(std::string_view data, Sink & sink)
{
    sink << data.size();
    sink(data);
    writePadding(data.size(), sink);
}

Sink & operator<<(Sink & sink, std::string_view s)
{
    writeString(s, sink);
    return sink;
}

template<class T>
void writeStrings(const T & ss, Sink & sink)
{
    sink << ss.size();
    for (auto & i : ss)
        sink << i;
}

Sink & operator<<(Sink & sink, const Strings & s)
{
    writeStrings(s, sink);
    return sink;
}

Sink & operator<<(Sink & sink, const StringSet & s)
{
    writeStrings(s, sink);
    return sink;
}

Sink & operator<<(Sink & sink, const Error & ex)
{
    return sink;
}

void readPadding(size_t len, Source & source)
{
    if (len % 8) {
        char zero[8];
        size_t n = 8 - (len % 8);
        source(zero, n);
        for (unsigned int i = 0; i < n; i++)
            if (zero[i])
                throw SerialisationError("non-zero padding");
    }
}

void StringSink::operator()(std::string_view data)
{
    s.append(data);
}

} // namespace nix
