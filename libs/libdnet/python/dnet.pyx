# cython: language_level=3, boundscheck=False
#
# dnet.pyx
#

"""dumb networking library

This module provides a simplified interface to several low-level
networking routines, including network address manipulation, kernel
arp(4) cache and route(4) table lookup and manipulation, network
firewalling, network interface lookup and manipulation, IP tunnelling,
and raw IP packet and Ethernet frame transmission.
"""

__author__ = 'Oliver Falk <oliver@linux-kernel.at>'
__copyright__ = 'Copyright (c) 2023-2024 Oliver Falk'
__license__ = 'BSD'
__url__ = 'https://github.com/ofalk/libdnet'
__version__ = '1.18.0'


cdef extern from "dnet.h":
    pass

cdef extern from "Python.h":
    object  PyBytes_FromStringAndSize(char *s, int len)
    int     PyBytes_Size(object o)
    int     PyObject_AsReadBuffer(object o, char **pp, int *lenp)
    int     PyLong_Check(object o)
    int     PyLong_Check(object o)
    long    PyLong_AsLong(object o)
    unsigned long PyLong_AsUnsignedLong(object o)

cdef extern from *:
    char   *malloc(int size)
    void    free(void *p)
    void   *memcpy(char *dst, char *src, int len)
    void   *memset(char *b, int c, int len)
    char   *strerror(int errnum)
    unsigned long htonl(unsigned long n)
    unsigned long ntohl(unsigned long n)

from dnet cimport *

cdef __memcpy(char *dst, object src, int n):
    if PyBytes_Size(src) != n:
        raise ValueError, "not a %d-byte binary string: %r" % (n, src)
    memcpy(dst, src, n)

cdef __oserror():
    cdef extern int errno = 0
    return strerror(errno)

def __iter_append(entry, l):
    l.append(entry)

#
# eth.h
#
cdef extern from *:
    ctypedef struct eth_t:
        int __xxx
    ctypedef struct eth_addr_t:
        char data[6]
    
    eth_t *eth_open(char *device)
    int    eth_get(eth_t *eth, eth_addr_t *ea)
    int    eth_set(eth_t *eth, eth_addr_t *ea)
    int    eth_send(eth_t *eth, char *buf, int len)
    eth_t *eth_close(eth_t *eth)

    char  *__eth_ntoa "eth_ntoa" (eth_addr_t *buf)
    int    __eth_aton "eth_aton" (char *src, eth_addr_t *dst)
    void   __eth_pack_hdr "eth_pack_hdr" (char *h,
               eth_addr_t dst, eth_addr_t src, int etype)

ETH_ADDR_LEN =	6
ETH_ADDR_BITS =	48
ETH_TYPE_LEN =	2
ETH_CRC_LEN =	4
ETH_HDR_LEN =	14

ETH_LEN_MIN =	64		# /* minimum frame length with CRC */
ETH_LEN_MAX =	1518		# /* maximum frame length with CRC */

ETH_MTU =	(ETH_LEN_MAX - ETH_HDR_LEN - ETH_CRC_LEN)
ETH_MIN =	(ETH_LEN_MIN - ETH_HDR_LEN - ETH_CRC_LEN)

ETH_TYPE_PUP =	0x0200		# /* PUP protocol */
ETH_TYPE_IP =	0x0800		# /* IP protocol */
ETH_TYPE_ARP =	0x0806		# /* address resolution protocol */
ETH_TYPE_REVARP=0x8035		# /* reverse addr resolution protocol */
ETH_TYPE_8021Q =0x8100		# /* IEEE 802.1Q VLAN tagging */
ETH_TYPE_IPV6 =	0x86DD		# /* IPv6 protocol */
ETH_TYPE_MPLS =	0x8847		# /* MPLS */
ETH_TYPE_MPLS_MCAST =	0x8848	# /* MPLS Multicast */
ETH_TYPE_PPPOEDISC =	0x8863	# /* PPP Over Ethernet Discovery Stage */
ETH_TYPE_PPPOE =	0x8864	# /* PPP Over Ethernet Session Stage */
ETH_TYPE_LOOPBACK =	0x9000	# /* used to test interfaces */

ETH_ADDR_UNSPEC =	PyBytes_FromStringAndSize("\x00\x00\x00\x00\x00\x00", 6)
ETH_ADDR_BROADCAST =	PyBytes_FromStringAndSize("\xff\xff\xff\xff\xff\xff", 6)

cdef class eth:
    """eth(device) -> Ethernet device object

    Open the specified Ethernet device for sending.
    """
    cdef eth_t *eth
    
    def __init__(self, device):
        self.eth = eth_open(device)
        if not self.eth:
            raise OSError, __oserror()
        
    def get(self):
        """Return the MAC address associated with the device as a
        binary string."""
        cdef eth_addr_t ea
        if eth_get(self.eth, &ea) < 0:
            raise OSError, __oserror()
        return PyBytes_FromStringAndSize(ea.data, 6)

    def set(self, value):
        """Set the MAC address for the device, returning 0 on success,
        -1 on failure.
        
        Arguments:
        eth_addr -- 6-byte binary string (e.g. '\\x00\\xde\\xad\\xbe\\xef\\x00')
        """
        cdef eth_addr_t ea
        __memcpy(ea.data, value, 6)
        if eth_set(self.eth, &ea) < 0:
            raise OSError, __oserror()

    def send(self, frame):
        """Send an Ethernet frame, returning the number of bytes sent
        or -1 on failure.
        
        Arguments:
        frame -- binary string representing an Ethernet frame
        """
        return eth_send(self.eth, frame, PyBytes_Size(frame))
    
    def __dealloc__(self):
        if self.eth:
            eth_close(self.eth)

def eth_ntoa(buf):
    """Convert an Ethernet MAC address from 6-byte packed binary string to
    a printable string ('00:de:ad:be:ef:00')."""
    cdef eth_addr_t ea
    __memcpy(ea.data, buf, 6)
    return __eth_ntoa(&ea)

def eth_aton(buf):
    """Convert an Ethernet MAC address from a printable string to a
    packed binary string ('\\x00\\xde\\xad\\xbe\\xef\\x00')."""
    cdef eth_addr_t ea
    if __eth_aton(buf, &ea) < 0:
        raise ValueError, "invalid Ethernet address"
    return PyBytes_FromStringAndSize(ea.data, 6)

def eth_pack_hdr(dst=ETH_ADDR_BROADCAST, src=ETH_ADDR_BROADCAST,
                 etype=ETH_TYPE_IP):
    """Return a packed binary string representing an Ethernet header.
	
    Keyword arguments:
    dst  -- destination address			(6-byte binary string)
    src  -- source address			(6-byte binary string)
    etype -- Ethernet payload type (ETH_TYPE_*)	(16-bit integer)
    """
    cdef char hdr[14]
    cdef eth_addr_t s, d
    __memcpy(s.data, src, 6)
    __memcpy(d.data, dst, 6)
    __eth_pack_hdr(hdr, d, s, etype)
    return PyBytes_FromStringAndSize(hdr, 14)

#
# ip.h
#
cdef extern from *:
    ctypedef struct ip_t:
        int __xxx
    ctypedef struct ip_addr_t:
        char data[4]

    ip_t *ip_open()
    int   ip_send(ip_t *ip, char *buf, int len)
    ip_t *ip_close(ip_t *ip)

    char *__ip_ntoa "ip_ntoa" (ip_addr_t *buf)
    int   __ip_aton "ip_aton" (char *src, ip_addr_t *dst)
    void  __ip_checksum "ip_checksum" (char *buf, int len)
    int   __ip_cksum_add "ip_cksum_add" (char *buf, int len, int sum)
    int   __ip_cksum_carry "ip_cksum_carry" (int sum)
    void  __ip_pack_hdr "ip_pack_hdr" (char *h, int tos, int len, int id,
              int off, int ttl, int p, ip_addr_t s, ip_addr_t d)

IP_ADDR_LEN =	4		# /* IP address length */
IP_ADDR_BITS =	32		# /* IP address bits */

IP_HDR_LEN =	20		# /* base IP header length */
IP_OPT_LEN =	2		# /* base IP option length */
IP_OPT_LEN_MAX =40
IP_HDR_LEN_MAX =(IP_HDR_LEN + IP_OPT_LEN_MAX)

IP_LEN_MAX =	65535
IP_LEN_MIN =	IP_HDR_LEN

IP_TOS_DEFAULT =0x00		# /* default */

IP_RF =		0x8000		# /* reserved */
IP_DF =		0x4000		# /* don't fragment */
IP_MF =		0x2000		# /* more fragments (not last frag) */
IP_OFFMASK =	0x1fff		# /* mask for fragment offset */

IP_TTL_DEFAULT =64		# /* default ttl, RFC 1122, RFC 1340 */
IP_TTL_MAX =	255		# /* maximum ttl */

IP_PROTO_IP =		0		# /* dummy for IP */
IP_PROTO_ICMP =		1		# /* ICMP */
IP_PROTO_IGMP =		2		# /* IGMP */
IP_PROTO_TCP =		6		# /* TCP */
IP_PROTO_UDP =		17		# /* UDP */
IP_PROTO_IPV6 =		41		# /* IPv6 */
IP_PROTO_GRE =		47		# /* General Routing Encap */
IP_PROTO_ESP =		50		# /* Encap Security Payload */
IP_PROTO_AH =		51		# /* Authentication Header */
IP_PROTO_ICMPV6 =	58		# /* ICMP for IPv6 */
IP_PROTO_RAW =		255		# /* Raw IP packets */
IP_PROTO_RESERVED =	IP_PROTO_RAW	# /* Reserved */
IP_PROTO_MAX =		255

IP_ADDR_ANY =		PyBytes_FromStringAndSize("\x00\x00\x00\x00", 4)
IP_ADDR_BROADCAST =	PyBytes_FromStringAndSize("\xff\xff\xff\xff", 4)
IP_ADDR_LOOPBACK =	PyBytes_FromStringAndSize("\x7f\x00\x00\x01", 4)
IP_ADDR_MCAST_ALL =	PyBytes_FromStringAndSize("\xe0\x00\x00\x01", 4)
IP_ADDR_MCAST_LOCAL =	PyBytes_FromStringAndSize("\xe0\x00\x00\xff", 4)

cdef class ip:
    """ip() -> Raw IP object

    Open a raw IP socket for sending.
    """
    cdef ip_t *ip

    def __init__(self):
        self.ip = ip_open()
        if not self.ip:
            raise OSError, __oserror()

    def send(self, pkt):
        """Send an IP packet, returning the number of bytes sent
        or -1 on failure.

        Arguments:
        pkt -- binary string representing an IP packet
        """
        return ip_send(self.ip, pkt, PyBytes_Size(pkt))

    def __dealloc__(self):
        if self.ip:
            ip_close(self.ip)

def ip_ntoa(buf):
    """Convert an IP address from a 4-byte packed binary string or
    integer to a printable string ('10.0.0.1')."""
    cdef ip_addr_t ia
    cdef unsigned int i

    if PyLong_Check(buf) or PyLong_Check(buf):
        i = ntohl(buf)
        memcpy(<char *>&ia, <char *>&i, 4)
    else:
        __memcpy(<char *>&ia, buf, 4)
    return __ip_ntoa(&ia)

def ip_aton(buf):
    """Convert an IP address from a printable string to a
    packed binary string ('\\x0a\\x00\\x00\\x01')."""
    cdef ip_addr_t ia
    if __ip_aton(buf, &ia) < 0:
        raise ValueError, "invalid IP address"
    return PyBytes_FromStringAndSize(<char *>&ia, 4)

def ip_checksum(pkt):
    """Return packed binary string representing an IP packet 
    with the IP and transport-layer checksums set.
    
    Arguments:
    pkt -- binary string representing an IP packet
    """
    cdef char buf[2048]
    cdef char *p
    cdef int n
    if PyObject_AsReadBuffer(pkt, &p, &n) == 0:
        if n < 2048:
            memcpy(buf, p, n)
            __ip_checksum(buf, n)
            return PyBytes_FromStringAndSize(buf, n)
        p = malloc(n)
        memcpy(p, pkt, n)
        __ip_checksum(p, n)
        s = PyBytes_FromStringAndSize(p, n)
        free(p)
        return s
    raise TypeError

def ip_cksum_add(buf, int sum):
    cdef char *p
    cdef int n
    if PyObject_AsReadBuffer(buf, &p, &n) == 0:
        return __ip_cksum_add(p, n, sum)
    else:
        raise TypeError

def ip_cksum_carry(int sum):
    return __ip_cksum_carry(sum)

def ip_pack_hdr(tos=IP_TOS_DEFAULT, len=IP_HDR_LEN, id=0, off=0,
                ttl=IP_TTL_DEFAULT, p=IP_PROTO_IP,
                src=IP_ADDR_ANY, dst=IP_ADDR_ANY):
    """Return a packed binary string representing an IP header.
    
    Keyword arguments:
    tos  -- type of service			(8-bit integer)
    len -- length (IP_HDR_LEN + payload)	(16-bit integer)
    id   -- packet ID				(16-bit integer)
    off  -- fragmentation offset		(16-bit integer)
    ttl  -- time-to-live			(8-bit integer)
    p    -- protocol (IP_PROTO_*)		(8-bit integer)
    src  -- source address			(4-byte binary string)
    dst  -- destination address			(4-byte binary string)
    """
    cdef char hdr[20]
    cdef ip_addr_t s, d
    __memcpy(<char *>&s, src, 4)
    __memcpy(<char *>&d, dst, 4)
    __ip_pack_hdr(hdr, tos, len, id, off, ttl, p, s, d)
    return PyBytes_FromStringAndSize(hdr, 20)

#
# ip6.h
#
cdef extern from *:
    ctypedef struct ip6_addr_t:
        char data[16]

    char *__ip6_ntoa "ip6_ntoa" (ip6_addr_t *buf)
    int   __ip6_aton "ip6_aton" (char *src, ip6_addr_t *dst)
    void  __ip6_checksum "ip6_checksum" (char *buf, int len)
    void  __ip6_pack_hdr "ip6_pack_hdr" (char *h, int fd, int fl, int plen,
              int nxt, int hlim, ip6_addr_t s, ip6_addr_t d)

IP6_ADDR_LEN =	16
IP6_ADDR_BITS =	128

IP6_HDR_LEN =	40		# /* IPv6 header length */
IP6_LEN_MIN =	IP6_HDR_LEN
IP6_LEN_MAX =	65535		# /* non-jumbo payload */

IP6_MTU_MIN =	1280		# /* minimum MTU (1024 + 256) */

IP6_HLIM_DEFAULT=64
IP6_HLIM_MAX =	255

IP6_ADDR_UNSPEC	= PyBytes_FromStringAndSize("\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00", 16)
IP6_ADDR_LOOPBACK = PyBytes_FromStringAndSize("\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x01", 16)

def ip6_ntoa(buf):
    """Convert an IPv6 address from a 16-byte packed binary string to a
    printable string ('10.0.0.1')."""
    cdef ip6_addr_t ia
    __memcpy(<char *>&ia, buf, 16)
    return __ip6_ntoa(&ia)

def ip6_aton(buf):
    """Convert an IPv6 address from a printable string to a
    packed binary string ('\\x0a\\x00\\x00\\x01')."""
    cdef ip6_addr_t ia
    if __ip6_aton(buf, &ia) < 0:
        raise ValueError, "invalid IPv6 address"
    return PyBytes_FromStringAndSize(<char *>&ia, 16)

def ip6_checksum(buf):
    """Return a packed binary string representing an IPv6 packet 
    with the IPv6 and transport-layer checksums set.

    Arguments:
    pkt -- binary string representing an IPv6 packet
    """
    __ip6_checksum(buf, PyBytes_Size(buf))
    return buf

def ip6_pack_hdr(fc=0, fl=0, plen=0, nxt=IP_PROTO_IPV6, hlim=IP6_HLIM_DEFAULT,
                 src=IP6_ADDR_UNSPEC, dst=IP6_ADDR_UNSPEC):
    """Return a packed binary string representing an IPv6 header.
    
    Keyword arguments:
    fc   -- flow class			(8-bit integer)
    fl   -- flow label			(20-bit integer)
    plen -- payload length		(16-bit integer)
    nxt  -- next header (IP_PROTO_*)	(8-bit integer)
    hlim -- hop limit			(8-bit integer)
    src  -- source address		(16-byte binary string)
    dst  -- destination address		(16-byte binary string)
    """
    cdef char hdr[40]
    cdef ip6_addr_t s, d
    __memcpy(<char *>&s, src, 16)
    __memcpy(<char *>&d, dst, 16)
    __ip6_pack_hdr(hdr, fc, fl, plen, nxt, hlim, s, d)
    return PyBytes_FromStringAndSize(hdr, 40)

#
# addr.h
#
cdef extern from *:
    cdef struct addr_t "addr":
        unsigned short addr_type
        unsigned short addr_bits
        char           addr_data8[16]
        unsigned long  addr_ip

    int   addr_cmp(addr_t *a, addr_t *b)
    int   addr_bcast(addr_t *a, addr_t *b)
    int   addr_net(addr_t *a, addr_t *b)
    char *addr_ntoa(addr_t *a)
    int   addr_aton(char *src, addr_t *dst)

ADDR_TYPE_NONE =	0
ADDR_TYPE_ETH =		1
ADDR_TYPE_IP =		2
ADDR_TYPE_IP6 =		3

cdef class addr:
    """addr(addrtxt=None, addrtype=ADDR_TYPE_NONE) -> network address object

    Create a network address object, optionally initialized from a
    human-readable Ethernet, IP, or IPv6 address string.
    """
    cdef addr_t _addr
    
    def __init__(self, addrtxt=None, addrtype=ADDR_TYPE_NONE):
        if addrtxt != None and addr_aton(addrtxt, &self._addr) < 0:
            if PyBytes_Size(addrtxt) == 4:
                self._addr.addr_type = ADDR_TYPE_IP
                self._addr.addr_bits = IP_ADDR_BITS
                self.ip = addrtxt
            else:
                raise ValueError, "invalid network address"
    
    property addrtype:
        """Address type (ADDR_TYPE_*) integer."""
        def __get__(self):
            return self._addr.addr_type
        def __set__(self, unsigned int value):
            if value > 0xffff: raise OverflowError
            self._addr.addr_type = value
    
    property bits:
        """Address bitlength integer."""
        def __get__(self):
            return self._addr.addr_bits
        def __set__(self, unsigned int value):
            if value > 0xffff: raise OverflowError
            self._addr.addr_bits = value

    property data:
        """Raw address string."""
        def __get__(self):
            if self._addr.addr_type == ADDR_TYPE_ETH:
                return self.eth
            elif self._addr.addr_type == ADDR_TYPE_IP:
                return self.ip
            elif self._addr.addr_type == ADDR_TYPE_IP6:
                return self.ip6
            else:
                raise ValueError, "invalid network address"
    
    property eth:
        """Ethernet MAC address as binary string."""
        def __get__(self):
            if self._addr.addr_type != ADDR_TYPE_ETH:
                raise ValueError, "non-Ethernet address"
            return PyBytes_FromStringAndSize(self._addr.addr_data8, 6)
        
        def __set__(self, value):
            if PyBytes_Size(value) != ETH_ADDR_LEN:
                raise ValueError, "not a 6-byte string"
            __memcpy(self._addr.addr_data8, value, 6)
            self._addr.addr_type = ADDR_TYPE_ETH
            self._addr.addr_bits = ETH_ADDR_BITS
    
    property ip:
        """IPv4 address as binary string."""
        def __get__(self):
            if self._addr.addr_type != ADDR_TYPE_IP:
                raise ValueError, "non-IP address"
            return PyBytes_FromStringAndSize(self._addr.addr_data8, 4)
        
        def __set__(self, value):
            # XXX - handle < 2.3, or else we'd use PyLong_AsUnsignedLongMask()
            if PyLong_Check(value):
                self._addr.addr_ip = htonl(PyLong_AsLong(value))
            elif PyLong_Check(value):
                self._addr.addr_ip = htonl(PyLong_AsUnsignedLong(value))
            elif PyBytes_Size(value) != IP_ADDR_LEN:
                raise ValueError, "not a 4-byte string"
            else:
                __memcpy(self._addr.addr_data8, value, 4)
            self._addr.addr_type = ADDR_TYPE_IP
            self._addr.addr_bits = IP_ADDR_BITS
    
    property ip6:
        """IPv6 address as binary string."""
        def __get__(self):
            if self._addr.addr_type != ADDR_TYPE_IP6:
                raise ValueError, "non-IPv6 address"
            return PyBytes_FromStringAndSize(self._addr.addr_data8, 16)
        
        def __set__(self, value):
            if PyBytes_Size(value) != IP6_ADDR_LEN:
                raise ValueError, "not a 16-byte string"
            __memcpy(self._addr.addr_data8, value, 16)
            self._addr.addr_type = ADDR_TYPE_IP6
            self._addr.addr_bits = IP6_ADDR_BITS

    def bcast(self):
        """Return an addr object for our broadcast address."""
        bcast = addr()
        addr_bcast(&self._addr, &(<addr>bcast)._addr)
        return bcast

    def net(self):
        """Return an addr object for our network address."""
        net = addr()
        addr_net(&self._addr, &(<addr>net)._addr)
        return net

    def __add__(self, other):
        # XXX - only handle IP for now...
        if PyLong_Check(self):
            x, y = other, self
        elif PyLong_Check(other):
            x, y = self, other
        else:
            raise NotImplementedError
        z = x.__copy__()
        (<addr>z)._addr.addr_ip = htonl(ntohl((<addr>x)._addr.addr_ip) + y)
        return z
        
    def __copy__(self):
        a = addr()
        (<addr>a)._addr = self._addr
        return a

    def __eq__(addr x, addr y):
        return addr_cmp(&x._addr, &y._addr) == 0

    def __ne__(addr x, addr y):
        return addr_cmp(&x._addr, &y._addr) != 0
    
    def __lt__(addr x, addr y):
        return addr_cmp(&x._addr, &y._addr) == -1
    
    def __gt__(addr x, addr y):
        return addr_cmp(&x._addr, &y._addr) == 1
    
    def __le__(addr x, addr y):
        return addr_cmp(&x._addr, &y._addr) != 1
    
    def __ge__(addr x, addr y):
        return addr_cmp(&x._addr, &y._addr) != -1

    def __contains__(self, addr other):
        cdef addr_t s1, s2, o1, o2
        if addr_net(&self._addr, &s1) != 0 or \
           addr_bcast(&self._addr, &s2) != 0 or \
           addr_net(&other._addr, &o1) != 0 or \
           addr_bcast(&other._addr, &o2) != 0:
            return 0
        return addr_cmp(&o1, &s1) >= 0 and addr_cmp(&o2, &s2) <= 0

    def __hash__(self):
        cdef long x, y, size
        if self._addr.addr_type == ADDR_TYPE_ETH: size = 6
        elif self._addr.addr_type == ADDR_TYPE_IP: size = 4
        elif self._addr.addr_type == ADDR_TYPE_IP6: size = 16
        x = 0x345678
        x = x ^ self._addr.addr_type
        x = x ^ self._addr.addr_bits
        y = self._addr.addr_data8[0] << 7
        for i from 0 < i < size:
            y = (1000003 * y) ^ self._addr.addr_data8[i]
        y = y ^ size
        if y == -1: y = -2
        x = x ^ y
        if x == -1: x = -2
        return x

    def __int__(self):
        if self._addr.addr_type != ADDR_TYPE_IP:
            raise NotImplementedError
        return ntohl(self._addr.addr_ip)
    
    def __long__(self):
        return self.__int__()
    
    def __iter__(self):
        cdef addr_t a, b
        if self._addr.addr_type != ADDR_TYPE_IP or \
           addr_net(&self._addr, &a) != 0 or \
           addr_bcast(&self._addr, &b) != 0:
            raise ValueError
        """XXX - i wish!
        for i in ntohl(a.addr_ip) <= i <= ntohl(b.addr_ip):
            next = addr()
            next._addr.addr_type = ADDR_TYPE_IP
            next._addr.addr_bits = IP_ADDR_BITS
            next._addr.addr_ip = htonl(i)
            yield next
        """
        return __addr_ip4_iter(a.addr_ip, b.addr_ip)
    
    def __repr__(self):
        cdef char *p
        p = addr_ntoa(&self._addr)
        if not p:
            return '<invalid network address>'
        return p

cdef class __addr_ip4_iter:
    cdef unsigned long cur	# XXX - HBO
    cdef unsigned long max	# XXX - HBO

    def __init__(self, cur, max):
        self.cur = ntohl(cur)
        self.max = ntohl(max)
    
    def __next__(self):
        cdef addr next
        if (self.cur <= self.max):
            next = addr()
            next._addr.addr_type = ADDR_TYPE_IP
            next._addr.addr_bits = IP_ADDR_BITS
            next._addr.addr_ip = htonl(self.cur)
            self.cur = self.cur + 1
            return next
        else:
            raise StopIteration

#
# arp.h
#
cdef extern from *:
    cdef struct arp_entry:
        addr_t arp_pa
        addr_t arp_ha
    ctypedef struct arp_t:
        int __xxx
    ctypedef int (*arp_handler)(arp_entry *entry, void *arg) except -1
    
    arp_t *arp_open()
    int    arp_add(arp_t *arp, arp_entry *entry)
    int    arp_delete(arp_t *arp, arp_entry *entry)
    int    arp_get(arp_t *arp, arp_entry *entry)
    int    arp_loop(arp_t *arp, arp_handler callback, void *arg)
    arp_t *arp_close(arp_t *arp)

    void   __arp_pack_hdr_ethip "arp_pack_hdr_ethip" (char *buf,
               int op, eth_addr_t sha, ip_addr_t spa,
               eth_addr_t dha, ip_addr_t dpa)

ARP_HDR_LEN =	8	# /* base ARP header length */
ARP_ETHIP_LEN =	20	# /* base ARP message length */

ARP_HRD_ETH = 	0x0001	# /* ethernet hardware */
ARP_HRD_IEEE802=0x0006	# /* IEEE 802 hardware */

ARP_PRO_IP =	0x0800	# /* IP protocol */

ARP_OP_REQUEST =	1	# /* request to resolve ha given pa */
ARP_OP_REPLY =		2	# /* response giving hardware address */
ARP_OP_REVREQUEST =	3	# /* request to resolve pa given ha */
ARP_OP_REVREPLY =	4	# /* response giving protocol address */

cdef int __arp_callback(arp_entry *entry, void *arg) except -1:
    f, a = <object>arg
    pa, ha = addr(), addr()
    (<addr>pa)._addr = entry.arp_pa
    (<addr>ha)._addr = entry.arp_ha
    ret = f((pa, ha), a)
    if not ret:
        ret = 0
    return ret

cdef class arp:
    """arp() -> ARP table object

    Open a handle to the system ARP table.
    """
    cdef arp_t *arp
    
    def __init__(self):
        self.arp = arp_open()
        if not self.arp:
            raise OSError, __oserror()

    def add(self, addr pa, addr ha):
        """Add an entry to the system ARP table.

        Arguments:
        pa -- ADDR_TYPE_IP network address object
        ha -- ADDR_TYPE_ETH network address object
        """
        cdef arp_entry entry
        entry.arp_pa = pa._addr
        entry.arp_ha = ha._addr
        if arp_add(self.arp, &entry) < 0:
            raise OSError, __oserror()

    def delete(self, addr pa):
        """Delete an entry from the system ARP table.

        Arguments:
        pa -- ADDR_TYPE_IP network address object
        """
        cdef arp_entry entry
        entry.arp_pa = pa._addr
        if arp_delete(self.arp, &entry) < 0:
            raise OSError, __oserror()
    
    def get(self, addr pa):
        """Return the hardware address for a given protocol address
        in the system ARP table.

        Arguments:
        pa -- ADDR_TYPE_IP network address object
        """
        cdef arp_entry entry
        entry.arp_pa = pa._addr
        if arp_get(self.arp, &entry) == 0:
            return addr(addr_ntoa(&entry.arp_ha))
        return None

    def loop(self, callback, arg=None):
        """Iterate over the system ARP table, invoking a user callback
        with each entry, returning the status of the callback routine.

        Keyword arguments:
        callback -- callback function with ((pa, ha), arg) prototype.
                    If this function returns a non-zero value, the loop
                    will break early.
        arg      -- optional callback argument
        """
        _arg = (callback, arg)
        return arp_loop(self.arp, __arp_callback, <void *>_arg)

    def __iter__(self):
        l = []
        self.loop(__iter_append, l)
        return iter(l)
    
    def __dealloc__(self):
        if self.arp:
            arp_close(self.arp)

def arp_pack_hdr_ethip(op=ARP_OP_REQUEST,
                       sha=ETH_ADDR_UNSPEC, spa=IP_ADDR_ANY,
                       dha=ETH_ADDR_UNSPEC, dpa=IP_ADDR_ANY):
    """Return a packed binary string representing an Ethernet/IP ARP message.
    
    Keyword arguments:
    op  -- operation (ARP_OP_*)			(16-bit integer)
    sha -- sender Ethernet address		(6-byte binary string)
    spa -- sender IP address			(4-byte binary string)
    dha -- destination Ethernet address		(6-byte binary string)
    dpa -- destination IP address		(4-byte binary string)
    """
    cdef char buf[28]
    cdef eth_addr_t sh, dh
    cdef ip_addr_t sp, dp
    __memcpy(sh.data, sha, 6)
    __memcpy(dh.data, dha, 6)
    __memcpy(<char *>&sp, spa, 4)
    __memcpy(<char *>&dp, dpa, 4)
    __arp_pack_hdr_ethip(buf, op, sh, sp, dh, dp)
    return PyBytes_FromStringAndSize(buf, 28)

#
# icmp.h
#
cdef extern from *:
    void __icmp_pack_hdr "icmp_pack_hdr" (char *hdr, int type, int code)
    
def icmp_pack_hdr(itype, code):
    """Return a packed binary string representing an ICMP header.

    Keyword arguments:
    itype -- ICMP type		(8-bit integer)
    code -- ICMP code		(8-bit integer)
    """
    cdef char buf[4]
    __icmp_pack_hdr(buf, itype, code)
    return PyBytes_FromStringAndSize(buf, sizeof(buf))

#
# tcp.h
#
cdef extern from *:
    void __tcp_pack_hdr "tcp_pack_hdr" (char *hdr,
        int sport, int dport, unsigned long seq, unsigned long ack, int flags, int win, int urp)

TCP_HDR_LEN =	20		# /* base TCP header length */

TH_FIN =	0x01		# /* end of data */
TH_SYN =	0x02		# /* synchronize sequence numbers */
TH_RST = 	0x04		# /* reset connection */
TH_PUSH =	0x08		# /* push */
TH_ACK =	0x10		# /* acknowledgement number set */
TH_URG =	0x20		# /* urgent pointer set */
TH_ECE =	0x40		# /* ECN echo, RFC 3168 */
TH_CWR =	0x80		# /* congestion window reduced */

TCP_PORT_MAX =	65535		# /* maximum port */
TCP_WIN_MAX =	65535		# /* maximum (unscaled) window */

TCP_OPT_EOL =		0	# /* end of option list */
TCP_OPT_NOP =		1	# /* no operation */
TCP_OPT_MSS =		2	# /* maximum segment size */
TCP_OPT_WSCALE =	3	# /* window scale factor, RFC 1072 */
TCP_OPT_SACKOK =	4	# /* SACK permitted, RFC 2018 */
TCP_OPT_SACK =		5	# /* SACK, RFC 2018 */
TCP_OPT_ECHO =		6	# /* echo (obsolete), RFC 1072 */
TCP_OPT_ECHOREPLY =	7	# /* echo reply (obsolete), RFC 1072 */
TCP_OPT_TIMESTAMP =	8	# /* timestamp, RFC 1323 */
TCP_OPT_POCONN =	9	# /* partial order conn, RFC 1693 */
TCP_OPT_POSVC =		10	# /* partial order service, RFC 1693 */
TCP_OPT_CC =		11	# /* connection count, RFC 1644 */
TCP_OPT_CCNEW =		12	# /* CC.NEW, RFC 1644 */
TCP_OPT_CCECHO =	13	# /* CC.ECHO, RFC 1644 */
TCP_OPT_ALTSUM =	14	# /* alt checksum request, RFC 1146 */
TCP_OPT_ALTSUMDATA =	15	# /* alt checksum data, RFC 1146 */
TCP_OPT_SKEETER =	16	# /* Skeeter */
TCP_OPT_BUBBA =		17	# /* Bubba */
TCP_OPT_TRAILSUM =	18	# /* trailer checksum */
TCP_OPT_MD5 =		19	# /* MD5 signature, RFC 2385 */
TCP_OPT_SCPS =		20	# /* SCPS capabilities */
TCP_OPT_SNACK =		21	# /* selective negative acks */
TCP_OPT_REC =		22	# /* record boundaries */
TCP_OPT_CORRUPT =	23	# /* corruption experienced */
TCP_OPT_SNAP =		24	# /* SNAP */
TCP_OPT_TCPCOMP =	26	# /* TCP compression filter */
TCP_OPT_MAX =		27

def tcp_pack_hdr(sport, dport, seq=1, ack=0, flags=TH_SYN,
                 win=TCP_WIN_MAX, urp=0):
    """Return a packed binary string representing a TCP header.

    Keyword arguments:
    sport -- source port		(16-bit integer)
    dport -- destination port		(16-bit integer)
    seq   -- sequence number		(32-bit integer)
    ack   -- acknowledgment number	(32-bit integer)
    flags -- control flags (TH_*)	(8-bit integer bitmask)
    win   -- window size		(16-bit integer)
    urp   -- urgent pointer		(16-bit integer)
    """
    cdef char buf[20]
    __tcp_pack_hdr(buf, sport, dport, seq, ack, flags, win, urp)
    return PyBytes_FromStringAndSize(buf, sizeof(buf))

#
# udp.h
#
cdef extern from *:
    void __udp_pack_hdr "udp_pack_hdr" (char *hdr, int sport, int dport, int ulen)

UDP_HDR_LEN =	8
UDP_PORT_MAX =	65535

def udp_pack_hdr(sport, dport, ulen=UDP_HDR_LEN):
    """Return a packed binary string representing a UDP header.

    Keyword arguments:
    sport -- source port		(16-bit integer)
    dport -- destination port		(16-bit integer)
    ulen  -- UDP header + data length	(16-bit integer)
    """
    cdef char buf[8]
    __udp_pack_hdr(buf, sport, dport, ulen)
    return PyBytes_FromStringAndSize(buf, sizeof(buf))

#
# intf.h
#
cdef extern from *:
    struct intf_entry:
        unsigned int	intf_len
        char		intf_name[16]
        unsigned short	intf_type
        unsigned short	intf_flags
        unsigned int	intf_mtu
        addr_t		intf_addr
        addr_t		intf_dst_addr
        addr_t		intf_link_addr
        unsigned int	intf_alias_num
        addr_t		intf_alias_addrs[8]	# XXX
    ctypedef struct intf_t:
        int __xxx
    ctypedef int (*intf_handler)(intf_entry *entry, void *arg) except -1
    
    intf_t *intf_open()
    int     intf_get(intf_t *intf, intf_entry *entry)
    int     intf_get_src(intf_t *intf, intf_entry *entry, addr_t *src)
    int     intf_get_dst(intf_t *intf, intf_entry *entry, addr_t *dst)
    int     intf_set(intf_t *intf, intf_entry *entry)
    int     intf_loop(intf_t *intf, intf_handler callback, void *arg)
    intf_t *intf_close(intf_t *intf)

INTF_TYPE_OTHER =	1	# /* other */
INTF_TYPE_ETH =		6	# /* Ethernet */
INTF_TYPE_LOOPBACK =	24	# /* software loopback */
INTF_TYPE_TUN =		53	# /* proprietary virtual/internal */

INTF_FLAG_UP =		0x01	# /* enable interface */
INTF_FLAG_LOOPBACK =	0x02	# /* is a loopback net (r/o) */
INTF_FLAG_POINTOPOINT =	0x04	# /* point-to-point link (r/o) */
INTF_FLAG_NOARP =	0x08	# /* disable ARP */
INTF_FLAG_BROADCAST =	0x10	# /* supports broadcast (r/o) */
INTF_FLAG_MULTICAST =	0x20	# /* supports multicast (r/o) */

cdef object ifent_to_dict(intf_entry *entry):
    d = {}
    d['name'] = entry.intf_name
    d['type'] = entry.intf_type
    d['flags'] = entry.intf_flags
    d['mtu'] = entry.intf_mtu
    if entry.intf_addr.addr_type != ADDR_TYPE_NONE:
        d['addr'] = addr(addr_ntoa(&entry.intf_addr))
    if entry.intf_dst_addr.addr_type != ADDR_TYPE_NONE:
        d['dst_addr'] = addr(addr_ntoa(&entry.intf_dst_addr))
    if entry.intf_link_addr.addr_type != ADDR_TYPE_NONE:
        d['link_addr'] = addr(addr_ntoa(&entry.intf_link_addr))
    if entry.intf_alias_num > 0:
        l = []
        for i from 0 <= i < entry.intf_alias_num:
            l.append(addr(addr_ntoa(&entry.intf_alias_addrs[i])))
        d['alias_addrs'] = l
    return d

cdef dict_to_ifent(object d, intf_entry *entry):
    s = d['name']
    strlcpy(entry.intf_name, s, 16)
    if 'flags' in d:
        entry.intf_flags = d['flags']
    if 'mtu' in d:
        entry.intf_mtu = d['mtu']
    if 'addr' in d:
        entry.intf_addr = (<addr>d['addr'])._addr
    if 'dst_addr' in d:
        entry.intf_dst_addr = (<addr>d['dst_addr'])._addr
    if 'link_addr' in d:
        entry.intf_link_addr = (<addr>d['link_addr'])._addr
    if 'alias_addrs' in d:
        entry.intf_alias_num = len(d['alias_addrs'])
        for i from 0 <= i < entry.intf_alias_num:
            entry.intf_alias_addrs[i] = (<addr>d['alias_addrs'][i])._addr

cdef int __intf_callback(intf_entry *entry, void *arg) except -1:
    f, a = <object>arg
    ret = f(ifent_to_dict(entry), a)
    if not ret:
        ret = 0
    return ret

cdef class intf:
    """intf() -> Interface table object

    Open a handle to the system network interface table.
    """
    cdef intf_t *intf

    def __init__(self):
        self.intf = intf_open()
        if not self.intf:
            raise OSError, __oserror()
    
    def get(self, name):
        """Return the configuration for a network interface as a dict.
        """
        cdef intf_entry *ifent
        cdef char buf[1024]
        ifent = <intf_entry *>buf
        ifent.intf_len = 1024
        strlcpy(ifent.intf_name, name, 16)
        if intf_get(self.intf, ifent) < 0:
            raise OSError, __oserror()
        return ifent_to_dict(ifent)

    def get_src(self, addr src):
        """Return the configuration for the interface whose primary address
        matches the specified source address.
        """
        cdef intf_entry *ifent
        cdef char buf[1024]
        ifent = <intf_entry *>buf
        ifent.intf_len = 1024
        if intf_get_src(self.intf, ifent, &src._addr) < 0:
            raise OSError, __oserror()
        return ifent_to_dict(ifent)
        
    def get_dst(self, addr dst):
        """Return the configuration for the best interface with which to
        reach the specified dst address.
        """
        cdef intf_entry *ifent
        cdef char buf[1024]
        ifent = <intf_entry *>buf
        ifent.intf_len = 1024
        if intf_get_dst(self.intf, ifent, &dst._addr) < 0:
            raise OSError, __oserror()
        return ifent_to_dict(ifent)
    
    def set(self, d):
        """Set the configuration for an interface from a dict.

        Dict values:
        name        -- name of interface to set		(string)
        flags       -- interface flags (INTF_FLAG_*)	(integer bitmask)
        mtu         -- interface MTU			(integer)
        addr        -- primary network address		(addr object)
        dst_addr    -- point-to-point dst address	(addr object)
        link_addr   -- link-layer address		(addr object)
        alias_addrs -- additional network addresses	(list of addr objects)
        """
        cdef intf_entry *ifent
        cdef char buf[1024]
        memset(buf, 0, sizeof(buf))
        ifent = <intf_entry *>buf
        ifent.intf_len = 1024
        dict_to_ifent(d, ifent)
        if intf_set(self.intf, ifent) < 0:
            raise OSError, __oserror()
    
    def loop(self, callback, arg=None):
        """Iterate over the system interface table, invoking a user callback
        with each entry, returning the status of the callback routine.

        Keyword arguments:
        callback -- callback function with (dict, arg) prototype.
                    If this function returns a non-zero value, the loop
                    will break early.
        arg      -- optional callback argument
        """
        _arg = (callback, arg)
        return intf_loop(self.intf, __intf_callback, <void *>_arg)

    def __iter__(self):
        l = []
        self.loop(__iter_append, l)
        return iter(l)
            
    def __dealloc__(self):
        if self.intf:
            intf_close(self.intf)

#
# route.h
#
cdef extern from *:
    cdef struct route_entry:
        addr_t route_dst
        addr_t route_gw
    ctypedef struct route_t:
        int __xxx
    ctypedef int (*route_handler)(route_entry *entry, void *arg) except -1
    
    route_t *route_open()
    int      route_add(route_t *route, route_entry *entry)
    int      route_delete(route_t *route, route_entry *entry)
    int      route_get(route_t *route, route_entry *entry)
    int      route_loop(route_t *route, route_handler callback, void *arg)
    route_t *route_close(route_t *route)

cdef int __route_callback(route_entry *entry, void *arg) except -1:
    f, a = <object>arg
    dst, gw = addr(), addr()
    (<addr>dst)._addr = entry.route_dst
    (<addr>gw)._addr = entry.route_gw
    ret = f((dst, gw), a)
    if not ret:
        ret = 0
    return ret

cdef class route:
    """route() -> Routing table object

    Open a handle to the system routing table.
    """
    cdef route_t *route
    
    def __init__(self):
        self.route = route_open()
        if not self.route:
            raise OSError, __oserror()

    def add(self, addr dst, addr gw):
        """Add an entry to the system routing table.

        Arguments:
        dst -- ADDR_TYPE_IP network address object
        gw -- ADDR_TYPE_IP network address object
        """
        cdef route_entry entry
        entry.route_dst = dst._addr
        entry.route_gw = gw._addr
        if route_add(self.route, &entry) < 0:
            raise OSError, __oserror()

    def delete(self, addr dst):
        """Delete an entry from the system routing table.

        Arguments:
        dst -- ADDR_TYPE_IP network address object
        """
        cdef route_entry entry
        entry.route_dst = dst._addr
        if route_delete(self.route, &entry) < 0:
            raise OSError, __oserror()
    
    def get(self, addr dst):
        """Return the hardware address for a given protocol address
        in the system routing table.

        Arguments:
        dst -- ADDR_TYPE_IP network address object
        """
        cdef route_entry entry
        entry.route_dst = dst._addr
        if route_get(self.route, &entry) == 0:
            return addr(addr_ntoa(&entry.route_gw))
        return None

    def loop(self, callback, arg=None):
        """Iterate over the system routing table, invoking a user callback
        with each entry, returning the status of the callback routine.

        Keyword arguments:
        callback -- callback function with ((dst, gw), arg) prototype.
                    If this function returns a non-zero value, the loop
                    will break early.
        arg      -- optional callback argument
        """
        _arg = (callback, arg)
        return route_loop(self.route, __route_callback, <void *>_arg)
    
    def __iter__(self):
        l = []
        self.loop(__iter_append, l)
        return iter(l)
    
    def __dealloc__(self):
        if self.route:
            route_close(self.route)

#
# fw.h
#
cdef extern from *:
    cdef struct fw_rule:
        char	fw_device[16]
        int	fw_op
        int	fw_dir
        int	fw_proto
        addr_t	fw_src
        addr_t	fw_dst
        int	fw_sport[2]
        int	fw_dport[2]
    
    ctypedef struct fw_t:
        int __xxx
    ctypedef int (*fw_handler)(fw_rule *rule, void *arg) except -1

    fw_t *fw_open()
    int	  fw_add(fw_t *f, fw_rule *rule)
    int	  fw_delete(fw_t *f, fw_rule *rule)
    int	  fw_loop(fw_t *f, fw_handler callback, void *arg)
    fw_t *fw_close(fw_t *f)

FW_OP_ALLOW =	1
FW_OP_BLOCK =	2

FW_DIR_IN =	1
FW_DIR_OUT =	2

cdef object rule_to_dict(fw_rule *rule):
    d = {}
    d['device'] = rule.fw_device
    d['op'] = rule.fw_op
    d['dir'] = rule.fw_dir
    if rule.fw_proto != 0:
        d['proto'] = rule.fw_proto
    if rule.fw_src.addr_type != ADDR_TYPE_NONE:
        d['src'] = addr(addr_ntoa(&rule.fw_src))
    if rule.fw_dst.addr_type != ADDR_TYPE_NONE:
        d['dst'] = addr(addr_ntoa(&rule.fw_dst))
    if not (rule.fw_sport[0] == 0 and rule.fw_sport[1] == 0):
        d['sport'] = [ rule.fw_sport[0], rule.fw_sport[1] ]
    if not (rule.fw_dport[0] == 0 and rule.fw_dport[1] == 0):
        d['dport'] = [ rule.fw_dport[0], rule.fw_dport[1] ]
    return d

cdef dict_to_rule(object d, fw_rule *rule):
    s = d['device']
    strlcpy(rule.fw_device, s, 16)
    rule.fw_op = d['op']
    rule.fw_dir = d['dir']
    if 'proto' in d:
        rule.fw_proto = d['proto']
        if rule.fw_proto == IP_PROTO_TCP or rule.fw_proto == IP_PROTO_UDP:
            rule.fw_sport[1] = 65535
            rule.fw_dport[1] = 65535
    if 'src' in d:
        rule.fw_src = (<addr>d['src'])._addr
    if 'dst' in d:
        rule.fw_dst = (<addr>d['dst'])._addr
    if 'sport' in d:
        rule.fw_sport[0] = d['sport'][0]
        rule.fw_sport[1] = d['sport'][1]
    if 'dport' in d:
        rule.fw_dport[0] = d['dport'][0]
        rule.fw_dport[1] = d['dport'][1]

cdef int __fw_callback(fw_rule *rule, void *arg) except -1:
    f, a = <object>arg
    ret = f(rule_to_dict(rule), a)
    if not ret:
        ret = 0
    return ret

cdef class fw:
    """fw() -> Firewall ruleset object
    
    Open a handle to the local network firewall configuration.
    """
    cdef fw_t *fw
    
    def __init__(self):
        self.fw = fw_open()
        if not self.fw:
            raise OSError, __oserror()

    def add(self, d):
        """Add a firewall rule specified as a dict.

        Dict values:
        device -- interface name			(string)
        op     -- operation (FW_OP_*)			(integer)
        dir    -- direction (FW_DIR_*)			(integer)
        proto  -- IP protocol (IP_PROTO_*)		(integer)
        src    -- source address / net			(addr object)
        dst    -- destination address / net		(addr object)
        sport  -- source port range or ICMP type/mask	(list of 2 integers)
        dport  -- dest port range or ICMP code/mask	(list of 2 integers)
        """
        cdef fw_rule rule
        memset(<char *>&rule, 0, sizeof(rule))
        dict_to_rule(d, &rule)
        if fw_add(self.fw, &rule) < 0:
            raise OSError, __oserror()

    def delete(self, d):
        """Delete a firewall rule specified as a dict."""
        cdef fw_rule rule
        memset(<char *>&rule, 0, sizeof(rule))
        dict_to_rule(d, &rule)
        if fw_delete(self.fw, &rule) < 0:
            raise OSError, __oserror()

    def loop(self, callback, arg=None):
        """Iterate over the local firewall ruleset, invoking a user callback
        with each entry, returning the status of the callback routine.

        Keyword arguments:
        callback -- callback function with (dict, arg) prototype.
                    If this function returns a non-zero value, the loop
                    will break early.
        arg      -- optional callback argument
        """
        _arg = (callback, arg)
        return fw_loop(self.fw, __fw_callback, <void *>_arg)

    def __iter__(self):
        l = []
        self.loop(__iter_append, l)
        return iter(l)
    
    def __dealloc__(self):
        if self.fw:
            fw_close(self.fw)

#
# rand.h
#
cdef extern from *:
    ctypedef struct rand_t:
        int __xxx
    
    rand_t *rand_open()
    int     rand_get(rand_t *rand, char *buf, int len)
    int     rand_set(rand_t *rand, char *seed, int len)
    int     rand_add(rand_t *rand, char *buf, int len)
    unsigned int     rand_uint8(rand_t *rand)
    unsigned int     rand_uint16(rand_t *rand)
    unsigned long     rand_uint32(rand_t *rand)
    rand_t *rand_close(rand_t *rand)

cdef class rand:
    """rand() -> Pseudo-random number generator

    Obtain a handle for fast, cryptographically strong pseudo-random
    number generation. The starting seed is derived from the system
    random data source device (if one exists), or from the current time
    and random stack contents.
    """
    cdef rand_t *rand

    def __init__(self):
        self.rand = rand_open()
        if not self.rand:
            raise OSError, __oserror()

    def get(self, len):
        """Return a string of random bytes.
        
        Arguments:
        len -- number of random bytes to generate
        """
        cdef char buf[1024]
        cdef char *p
        if len <= 1024:
            rand_get(self.rand, buf, len)
            return PyBytes_FromStringAndSize(buf, len)
        p = malloc(len)
        rand_get(self.rand, p, len)
        s = PyBytes_FromStringAndSize(p, len)
        free(p)
        return s
    
    def set(self, buf):
        """Initialize the PRNG from a known seed.
        
        Arguments:
        string -- binary string seed value
        """
        rand_set(self.rand, buf, PyBytes_Size(buf))
        
    def add(self, buf):
        """Add additional entropy into the PRNG mix.

        Arguments:
        string -- binary string
        """
        rand_add(self.rand, buf, PyBytes_Size(buf))
    
    def uint8(self):
        """Return a random 8-bit integer."""
        return rand_uint8(self.rand)
    
    def uint16(self):
        """Return a random 16-bit integer."""
        return rand_uint16(self.rand)
    
    def uint32(self):
        """Return a random 32-bit integer."""
        return rand_uint32(self.rand)

    def xrange(self, start, stop=None):
        """xrange([start,] stop) -> xrange object

        Return a random permutation iterator to walk an unsigned integer range,
        like xrange().
        """
        if stop == None:
            return __rand_xrange(self, 0, start)
        else:
            return __rand_xrange(self, start, stop)
    
    def __dealloc__(self):
        if self.rand:
            rand_close(self.rand)

# Modified (variable block length) TEA by Niels Provos <provos@monkey.org>
cdef enum:
    TEADELTA	 = 0x9e3779b9
    TEAROUNDS	 = 32
    TEASBOXSIZE	 = 128
    TEASBOXSHIFT = 7

cdef class __rand_xrange:
    cdef rand_t *rand
    cdef unsigned long cur, enc, max, mask, start, sboxmask
    cdef unsigned int sbox[128] # TEASBOXSIZE
    cdef int left, right, kshift
    
    def __init__(self, r, start, stop):
        cdef unsigned int bits
        
        self.rand = (<rand>r).rand
        if PyLong_Check(start):
            self.start = PyLong_AsLong(start)
        elif PyLong_Check(start):
            self.start = PyLong_AsUnsignedLong(start)
        else:
            raise TypeError, 'start must be an integer'
        
        if PyLong_Check(start):
            self.max = PyLong_AsLong(stop) - self.start
        elif PyLong_Check(start):
            self.max = PyLong_AsUnsignedLong(stop) - self.start
        else:
            raise TypeError, 'stop must be an integer'
        
        # XXX - permute range once only!
        rand_get(self.rand, <char *>self.sbox, sizeof(self.sbox))
        
        bits = 0
        while self.max > (1 << bits):
            bits = bits + 1
        
        self.left = int(bits / 2)
        self.right = bits - self.left
        self.mask = (1 << bits) - 1

        if TEASBOXSIZE < (1 << self.left):
            self.sboxmask = TEASBOXSIZE - 1
            self.kshift = TEASBOXSHIFT
        else:
            self.sboxmask = (1 << self.left) - 1
            self.kshift = self.left

    def __iter__(self):
        self.cur = self.enc = 0
        # XXX - rewind iterator, but do not permute range again!
        return self

    def __len__(self):
        return self.max
    
    def __next__(self):
        cdef unsigned long c, sum
        
        if self.cur == self.max:
            raise StopIteration
        self.cur = self.cur + 1
        while 1:
            c = self.enc
            self.enc = self.enc + 1
            sum = 0
            for i from 0 < i < TEAROUNDS:
                sum = sum + TEADELTA
                c = c ^ (self.sbox[(c ^ sum) & self.sboxmask] << self.kshift)
                c = c + sum
                c = c & self.mask
                c = ((c << self.left) | (c >> self.right)) & self.mask
            if c < self.max:
                break
        return self.start + c

#
# tun.h
#
cdef extern from *:
    ctypedef struct tun_t:
        int __xxx
    
    tun_t *tun_open(addr_t *src, addr_t *dst, int mtu)
    int    tun_fileno(tun_t *tun)
    char  *tun_name(tun_t *tun)
    int    tun_send(tun_t *tun, char *buf, int size)
    int    tun_recv(tun_t *tun, char *buf, int size)
    tun_t *tun_close(tun_t *tun)

cdef class tun:
    """tun(src, dst[, mtu]) -> Network tunnel interface handle
    
    Obtain a handle to a network tunnel interface, to which packets
    destined for dst are delivered (with source addresses rewritten to
    src), where they may be read by a userland process and processed
    as desired. Packets written back to the handle are injected into
    the kernel networking subsystem.
    """
    cdef tun_t *tun
    cdef char *buf
    cdef int mtu

    def __init__(self, addr src, addr dst, mtu=1500):
        self.tun = tun_open(&src._addr, &dst._addr, mtu)
        self.mtu = mtu
        if not self.tun:
            raise OSError, __oserror()
        self.buf = malloc(mtu)

    property name:
        """Tunnel interface name."""
        def __get__(self):
            return tun_name(self.tun)

    property fd:
        """File descriptor for tunnel handle."""
        def __get__(self):
            return tun_fileno(self.tun)

    def fileno(self):
        """Return file descriptor for tunnel handle."""
        return tun_fileno(self.tun)
    
    def send(self, pkt):
        """Send an IP packet, returning the number of bytes sent
        or -1 on failure.

        Arguments:
        pkt -- binary string representing an IP packet
        """
        return tun_send(self.tun, pkt, PyBytes_Size(pkt))

    def recv(self):
        """Return the next packet delivered to the tunnel interface."""
        cdef int n
        n = tun_recv(self.tun, self.buf, self.mtu)
        if n < 0:
            raise OSError, __oserror()
        return PyBytes_FromStringAndSize(self.buf, n)

    def close(self):
        self.tun = tun_close(self.tun)
    
    def __dealloc__(self):
        if self.buf:
            free(self.buf)
        if self.tun:
            tun_close(self.tun)
