#include "mdnsd.h"
#include <string.h>
#include <stdlib.h>
#ifdef WIN32
// for the inet related functions
#include <ws2tcpip.h>
#else
#include <netinet/in.h>
#include <arpa/inet.h>
#include <sys/time.h>
#endif

#define bzero(b,len) (memset((b), '\0', (len)), (void) 0)

// size of query/publish hashes
#define SPRIME 108
// size of cache hash
#define LPRIME 1009
// brute force garbage cleanup frequency, rarely needed (daily default)
#define GC 86400

#ifdef WIN32
//#include "stdafx.h"
#include <time.h>
#include <windows.h>

const __int64 DELTA_EPOCH_IN_MICROSECS= 11644473600000000;

/* IN UNIX the use of the timezone struct is obsolete;
 I don't know why you use it. See http://linux.about.com/od/commands/l/blcmdl2_gettime.htm
 But if you want to use this structure to know about GMT(UTC) diffrence from your local time
 it will be next: tz_minuteswest is the real diffrence in minutes from GMT(UTC) and a tz_dsttime is a flag
 indicates whether daylight is now in use
*/
struct timezone2
{
  __int32  tz_minuteswest; /* minutes W of Greenwich */
  BOOL  tz_dsttime;     /* type of dst correction */
};

struct timeval2 {
__int32    tv_sec;         /* seconds */
__int32    tv_usec;        /* microseconds */
};

int mdnsd_gettimeofday(struct timeval *tv/*in*/, struct timezone2 *tz/*in*/)
{
  FILETIME ft;
  __int64 tmpres = 0;
  TIME_ZONE_INFORMATION tz_winapi;
  int rez=0;

   ZeroMemory(&ft,sizeof(ft));
   ZeroMemory(&tz_winapi,sizeof(tz_winapi));

    GetSystemTimeAsFileTime(&ft);

    tmpres = ft.dwHighDateTime;
    tmpres <<= 32;
    tmpres |= ft.dwLowDateTime;

    /*converting file time to unix epoch*/
    tmpres /= 10;  /*convert into microseconds*/
    tmpres -= DELTA_EPOCH_IN_MICROSECS;
    tv->tv_sec = (__int32)(tmpres*0.000001);
    tv->tv_usec =(tmpres%1000000);


    if(!tz)
        return 0;

    //_tzset(),don't work properly, so we use GetTimeZoneInformation
    rez=GetTimeZoneInformation(&tz_winapi);
    tz->tz_dsttime=(rez==2)?TRUE:FALSE;
    tz->tz_minuteswest = tz_winapi.Bias + ((rez==2)?tz_winapi.DaylightBias:0);

  return 0;
}
#else
#define mdnsd_gettimeofday gettimeofday
#endif


/* messy, but it's the best/simplest balance I can find at the moment
Some internal data types, and a few hashes: querys, answers, cached, and records (published, unique and shared)
Each type has different semantics for processing, both for timeouts, incoming, and outgoing I/O
They inter-relate too, like records affect the querys they are relevant to
Nice things about MDNS: we only publish once (and then ask asked), and only query once, then just expire records we've got cached
*/

struct query
{
    char *name;
    int type;
    unsigned long int nexttry;
    int tries;
    int (*answer)(mdnsda, void *);
    void *arg;
    struct query *next, *list;
};

struct unicast
{
    int id;
    unsigned long int to;
    unsigned short int port;
    mdnsdr r;
    struct unicast *next;
};

struct cached
{
    struct mdnsda_struct rr;
    struct query *q;
    struct cached *next;
};

struct mdnsdr_struct
{
    struct mdnsda_struct rr;
    char unique; // # of checks performed to ensure
    int tries;
    void (*conflict)(char *, int, void *);
    void *arg;
    struct mdnsdr_struct *next, *list;
};

struct mdnsd_struct
{
    char shutdown;
    unsigned long int expireall, checkqlist;
    struct timeval now, sleep, pause, probe, publish;
    int class, frame;
    struct cached *cache[LPRIME];
    struct mdnsdr_struct *published[SPRIME], *probing, *a_now, *a_pause, *a_publish;
    struct unicast *uanswers;
    struct query *queries[SPRIME], *qlist;
};

int _namehash(const char *s)
{
    const unsigned char *name = (const unsigned char *)s;
    unsigned long h = 0, g;

    while (*name)
    { /* do some fancy bitwanking on the string */
        h = (h << 4) + (unsigned long)(*name++);
        if ((g = (h & 0xF0000000UL))!=0)
            h ^= (g >> 24);
        h &= ~g;
    }

    return (int)h;
}

// basic linked list and hash primitives
struct query *_q_next(mdnsd d, struct query *q, char *host, int type)
{
    if(q == 0) q = d->queries[_namehash(host) % SPRIME];
    else q = q->next;
    for(;q != 0; q = q->next)
        if(q->type == type && strcmp(q->name, host) == 0)
            return q;
    return 0;
}
struct cached *_c_next(mdnsd d, struct cached *c, char *host, int type)
{
    if(c == 0) c = d->cache[_namehash(host) % LPRIME];
    else c = c->next;
    for(;c != 0; c = c->next)
        if((type == c->rr.type || type == 255) && strcmp(c->rr.name, host) == 0)
            return c;
    return 0;
}
mdnsdr _r_next(mdnsd d, mdnsdr r, char *host, int type)
{
    if(r == 0) r = d->published[_namehash(host) % SPRIME];
    else r = r->next;
    for(;r != 0; r = r->next)
        if(type == r->rr.type && strcmp(r->rr.name, host) == 0)
            return r;
    return 0;
}

int _rr_len(mdnsda rr)
{
    int len = 12; // name is always compressed (dup of earlier), plus normal stuff
    if(rr->rdata) len += rr->rdlen;
    if(rr->rdname) len += strlen(rr->rdname); // worst case
    if(rr->ip) len += 4;
    if(rr->type == QTYPE_PTR) len += 6; // srv record stuff
    return len;
}

int _a_match(struct resource *r, mdnsda a)
{ // compares new rdata with known a, painfully
    if(strcmp(r->name,a->name) || r->type != a->type) return 0;
    if(r->type == QTYPE_SRV && !strcmp(r->known.srv.name,a->rdname) && a->srv.port == r->known.srv.port && a->srv.weight == r->known.srv.weight && a->srv.priority == r->known.srv.priority) return 1;
    if((r->type == QTYPE_PTR || r->type == QTYPE_NS || r->type == QTYPE_CNAME) && !strcmp(a->rdname,r->known.ns.name)) return 1;
    if(r->rdlength == a->rdlen && !memcmp(r->rdata,a->rdata,r->rdlength)) return 1;
    return 0;
}

// compare time values easily
int _tvdiff(struct timeval old, struct timeval new)
{
    int udiff = 0;
    if(old.tv_sec != new.tv_sec) udiff = (new.tv_sec - old.tv_sec) * 1000000;
    return (new.tv_usec - old.tv_usec) + udiff;
}

// make sure not already on the list, then insert
void _r_push(mdnsdr *list, mdnsdr r)
{
    mdnsdr cur;
    for(cur = *list; cur != 0; cur = cur->list)
        if(cur == r) return;
    r->list = *list;
    *list = r;
}

// set this r to probing, set next probe time
void _r_probe(mdnsd d, mdnsdr r)
{
}

// force any r out right away, if valid
void _r_publish(mdnsd d, mdnsdr r)
{
    if(r->unique && r->unique < 5) return; // probing already
    r->tries = 0;
    d->publish.tv_sec = d->now.tv_sec; d->publish.tv_usec = d->now.tv_usec;
    _r_push(&d->a_publish,r);
}

// send r out asap
void _r_send(mdnsd d, mdnsdr r)
{
    if(r->tries < 4)
    { // being published, make sure that happens soon
        d->publish.tv_sec = d->now.tv_sec; d->publish.tv_usec = d->now.tv_usec;
        return;
    }
    if(r->unique)
    { // known unique ones can be sent asap
        _r_push(&d->a_now,r);
        return;
    }
    // set d->pause.tv_usec to random 20-120 msec
    d->pause.tv_sec = d->now.tv_sec;
    d->pause.tv_usec = d->now.tv_usec + (d->now.tv_usec % 100) + 20;
    _r_push(&d->a_pause,r);
}

// create generic unicast response struct
void _u_push(mdnsd d, mdnsdr r, int id, unsigned long int to, unsigned short int port)
{
    struct unicast *u;
    u = (struct unicast *)malloc(sizeof(struct unicast));
    bzero(u,sizeof(struct unicast));
    u->r = r;
    u->id = id;
    u->to = to;
    u->port = port;
    u->next = d->uanswers;
    d->uanswers = u;
}

void _q_reset(mdnsd d, struct query *q)
{
    struct cached *cur = 0;
    q->nexttry = 0;
    q->tries = 0;
    while(cur = _c_next(d,cur,q->name,q->type))
        if(q->nexttry == 0 || cur->rr.ttl - 7 < q->nexttry) q->nexttry = cur->rr.ttl - 7;
    if(q->nexttry != 0 && q->nexttry < d->checkqlist) d->checkqlist = q->nexttry;
}

void _q_done(mdnsd d, struct query *q)
{ // no more query, update all it's cached entries, remove from lists
    struct cached *c = 0;
    struct query *cur;
    int i = _namehash(q->name) % LPRIME;
    while(c = _c_next(d,c,q->name,q->type)) c->q = 0;
    if(d->qlist == q) d->qlist = q->list;
    else {
        for(cur=d->qlist;cur->list != q;cur = cur->list);
        cur->list = q->list;
    }
    if(d->queries[i] == q) d->queries[i] = q->next;
    else {
        for(cur=d->queries[i];cur->next != q;cur = cur->next);
        cur->next = q->next;
    }
    free(q->name);
    free(q);
}

void _r_done(mdnsd d, mdnsdr r)
{ // buh-bye, remove from hash and free
    mdnsdr cur = 0;
    int i = _namehash(r->rr.name) % SPRIME;
    if(d->published[i] == r) d->published[i] = r->next;
    else {
        for(cur=d->published[i];cur && cur->next != r;cur = cur->next);
        if(cur) cur->next = r->next;
    }
    free(r->rr.name);
    free(r->rr.rdata);
    free(r->rr.rdname);
    free(r);
}

void _q_answer(mdnsd d, struct cached *c)
{ // call the answer function with this cached entry
    if(c->rr.ttl <= d->now.tv_sec) c->rr.ttl = 0;
    if(c->q->answer(&c->rr,c->q->arg) == -1) _q_done(d, c->q);
}

void _conflict(mdnsd d, mdnsdr r)
{
    r->conflict(r->rr.name,r->rr.type,r->arg);
    mdnsd_done(d,r);
}

void _c_expire(mdnsd d, struct cached **list)
{ // expire any old entries in this list
    struct cached *next, *cur = *list, *last = 0;
    while(cur != 0)
    {
        next = cur->next;
        if(d->now.tv_sec >= cur->rr.ttl)
        {
            if(last) last->next = next;
            if(*list == cur) *list = next; // update list pointer if the first one expired
            if(cur->q) _q_answer(d,cur);
            free(cur->rr.name);
            free(cur->rr.rdata);
            free(cur->rr.rdname);
            free(cur);
        }else{
            last = cur;
        }
        cur = next;
    }
}

// brute force expire any old cached records
void _gc(mdnsd d)
{
    int i;
    for(i=0;i<LPRIME;i++)
        if(d->cache[i]) _c_expire(d,&d->cache[i]);
    d->expireall = d->now.tv_sec + GC;
}

void _cache(mdnsd d, struct resource *r)
{
    struct cached *c = 0;
    int i = _namehash(r->name) % LPRIME;

    if(r->rr_class == 32768 + d->class)
    { // cache flush
        while(c = _c_next(d,c,r->name,r->type)) c->rr.ttl = 0;
        _c_expire(d,&d->cache[i]);
    }

    if(r->ttl == 0)
    { // process deletes
        while(c = _c_next(d,c,r->name,r->type))
            if(_a_match(r,&c->rr))
	      c->rr.ttl = 0;

	_c_expire(d,&d->cache[i]);

        return;
    }

    c = (struct cached *)malloc(sizeof(struct cached));
    bzero(c,sizeof(struct cached));
    c->rr.name = strdup(r->name);
    c->rr.type = r->type;
    c->rr.ttl = d->now.tv_sec + (r->ttl / 2) + 8; // XXX hack for now, BAD SPEC, start retrying just after half-waypoint, then expire
    c->rr.rdlen = r->rdlength;
    c->rr.rdata = (unsigned char *)malloc(r->rdlength);
    memcpy(c->rr.rdata,r->rdata,r->rdlength);
    switch(r->type)
    {
    case QTYPE_A:
        c->rr.ip = r->known.a.ip;
        break;
    case QTYPE_NS:
    case QTYPE_CNAME:
    case QTYPE_PTR:
        c->rr.rdname = strdup(r->known.ns.name);
        break;
    case QTYPE_SRV:
        c->rr.rdname = strdup(r->known.srv.name);
        c->rr.srv.port = r->known.srv.port;
        c->rr.srv.weight = r->known.srv.weight;
        c->rr.srv.priority = r->known.srv.priority;
        break;
    }
    c->next = d->cache[i];
    d->cache[i] = c;
    if(c->q = _q_next(d, 0, r->name, r->type))
        _q_answer(d,c);
}

void _a_copy(struct message *m, mdnsda a)
{ // copy the data bits only
    if(a->rdata) { message_rdata_raw(m, a->rdata, a->rdlen); return; }
    if(a->ip) message_rdata_long(m, a->ip);
    if(a->type == QTYPE_SRV) message_rdata_srv(m, a->srv.priority, a->srv.weight, a->srv.port, a->rdname);
    else if(a->rdname) message_rdata_name(m, a->rdname);
}

int _r_out(mdnsd d, struct message *m, mdnsdr *list)
{ // copy a published record into an outgoing message
    mdnsdr r, next;
    int ret = 0;
    while((r = *list) != 0 && message_packet_len(m) + _rr_len(&r->rr) < d->frame)
    {
        *list = r->list;
        ret++;
        if(r->unique)
            message_an(m, r->rr.name, r->rr.type, d->class + 32768, r->rr.ttl);
        else
            message_an(m, r->rr.name, r->rr.type, d->class, r->rr.ttl);
        _a_copy(m, &r->rr);
        if(r->rr.ttl == 0) _r_done(d,r);
    }
    return ret;
}


mdnsd mdnsd_new(int class, int frame)
{
    int i;
    mdnsd d;
    d = (mdnsd)malloc(sizeof(struct mdnsd_struct));
    bzero(d,sizeof(struct mdnsd_struct));
    mdnsd_gettimeofday(&d->now,0);
    d->expireall = d->now.tv_sec + GC;
    d->class = class;
    d->frame = frame;
    return d;
}

void mdnsd_shutdown(mdnsd d)
{ // shutting down, zero out ttl and push out all records
    int i;
    mdnsdr cur,next;
    d->a_now = 0;
    for(i=0;i<SPRIME;i++)
        for(cur = d->published[i]; cur != 0;)
        {
            next = cur->next;
            cur->rr.ttl = 0;
            cur->list = d->a_now;
            d->a_now = cur;
            cur = next;
        }
    d->shutdown = 1;
}

void mdnsd_flush(mdnsd d)
{
    // set all querys to 0 tries
    // free whole cache
    // set all mdnsdr to probing
    // reset all answer lists
}

void mdnsd_free(mdnsd d)
{
    int i;
    // loop through all hashes, free everything
    // free answers if any
    free(d);
}

void mdnsd_in(mdnsd d, struct message *m, unsigned long int ip, unsigned short int port)
{
    int i, j;
    mdnsdr r = 0;

    if(d->shutdown) return;

    mdnsd_gettimeofday(&d->now,0);

    if(m->header.qr == 0)
    {
        for(i=0;i<m->qdcount;i++)
        { // process each query
            if(m->qd[i].rr_class != d->class || (r = _r_next(d,0,m->qd[i].name,m->qd[i].type)) == 0) continue;

            // send the matching unicast reply
            if(port != 5353) _u_push(d,r,m->id,ip,port);

            for(;r != 0; r = _r_next(d,r,m->qd[i].name,m->qd[i].type))
            { // check all of our potential answers
                if(r->unique && r->unique < 5)
                { // probing state, check for conflicts
                    for(j=0;j<m->nscount;j++)
                    { // check all to-be answers against our own
                        if(m->qd[i].type != m->an[j].type || strcmp(m->qd[i].name,m->an[j].name)) continue;
                        if(!_a_match(&m->an[j],&r->rr)) _conflict(d,r); // this answer isn't ours, conflict!
                    }
                    continue;
                }
                for(j=0;j<m->ancount;j++)
                { // check the known answers for this question
                    if(m->qd[i].type != m->an[j].type || strcmp(m->qd[i].name,m->an[j].name)) continue;
                    if(_a_match(&m->an[j],&r->rr)) break; // they already have this answer
                }
                if(j == m->ancount) _r_send(d,r);
            }
        }
        return;
    }

    for(i=0;i<m->ancount;i++)
    { // process each answer, check for a conflict, and cache
        if((r = _r_next(d,0,m->an[i].name,m->an[i].type)) != 0 && r->unique && _a_match(&m->an[i],&r->rr) == 0) _conflict(d,r);
        _cache(d,&m->an[i]);
    }
}

int mdnsd_out(mdnsd d, struct message *m, unsigned long int *ip, unsigned short int *port)
{
    mdnsdr r;
    int ret = 0;

    mdnsd_gettimeofday(&d->now,0);
    bzero(m,sizeof(struct message));

    // defaults, multicast
    *port = htons(5353);
    *ip = inet_addr("224.0.0.251");
    m->header.qr = 1;
    m->header.aa = 1;

    if(d->uanswers)
    { // send out individual unicast answers
        struct unicast *u = d->uanswers;
        d->uanswers = u->next;
        *port = u->port;
        *ip = u->to;
        m->id = u->id;
        message_qd(m, u->r->rr.name, u->r->rr.type, d->class);
        message_an(m, u->r->rr.name, u->r->rr.type, d->class, u->r->rr.ttl);
        _a_copy(m, &u->r->rr);
        free(u);
        return 1;
    }

//printf("OUT: probing %X now %X pause %X publish %X\n",d->probing,d->a_now,d->a_pause,d->a_publish);

    // accumulate any immediate responses
    if(d->a_now) ret += _r_out(d, m, &d->a_now);

    if(d->a_publish && _tvdiff(d->now,d->publish) <= 0)
    { // check to see if it's time to send the publish retries (and unlink if done)
        mdnsdr next, cur = d->a_publish, last = 0;
        while(cur && message_packet_len(m) + _rr_len(&cur->rr) < d->frame)
        {
            next = cur->list;
            ret++; cur->tries++;
            if(cur->unique)
                message_an(m, cur->rr.name, cur->rr.type, d->class + 32768, cur->rr.ttl);
            else
                message_an(m, cur->rr.name, cur->rr.type, d->class, cur->rr.ttl);
            _a_copy(m, &cur->rr);
            if(cur->rr.ttl != 0 && cur->tries < 4)
            {
                last = cur;
                cur = next;
                continue;
            }
            if(d->a_publish == cur) d->a_publish = next;
            if(last) last->list = next;
            if(cur->rr.ttl == 0) _r_done(d,cur);
            cur = next;
        }
        if(d->a_publish)
        {
            d->publish.tv_sec = d->now.tv_sec + 2;
            d->publish.tv_usec = d->now.tv_usec;
        }
    }

    // if we're in shutdown, we're done
    if(d->shutdown) return ret;

    // check if a_pause is ready
    if(d->a_pause && _tvdiff(d->now, d->pause) <= 0) ret += _r_out(d, m, &d->a_pause);

    // now process questions
    if(ret) return ret;
    m->header.qr = 0;
    m->header.aa = 0;

    if(d->probing && _tvdiff(d->now,d->probe) <= 0)
    {
        mdnsdr last = 0;
        for(r = d->probing; r != 0;)
        { // scan probe list to ask questions and process published
            if(r->unique == 4)
            { // done probing, publish
                mdnsdr next = r->list;
                if(d->probing == r)
                    d->probing = r->list;
                else
                    last->list = r->list;
                r->list = 0;
                r->unique = 5;
                _r_publish(d,r);
                r = next;
                continue;
            }
            message_qd(m, r->rr.name, r->rr.type, d->class);
            last = r;
            r = r->list;
        }
        for(r = d->probing; r != 0; last = r, r = r->list)
        { // scan probe list again to append our to-be answers
            r->unique++;
            message_ns(m, r->rr.name, r->rr.type, d->class, r->rr.ttl);
            _a_copy(m, &r->rr);
            ret++;
        }
        if(ret)
        { // process probes again in the future
            d->probe.tv_sec = d->now.tv_sec;
            d->probe.tv_usec = d->now.tv_usec + 250000;
            return ret;
        }
    }

    if(d->checkqlist && d->now.tv_sec >= d->checkqlist)
    { // process qlist for retries or expirations
        struct query *q;
        struct cached *c;
        unsigned long int nextbest = 0;

        // ask questions first, track nextbest time
        for(q = d->qlist; q != 0; q = q->list)
            if(q->nexttry > 0 && q->nexttry <= d->now.tv_sec && q->tries < 3)
                message_qd(m,q->name,q->type,d->class);
            else if(q->nexttry > 0 && (nextbest == 0 || q->nexttry < nextbest))
                nextbest = q->nexttry;

        // include known answers, update questions
        for(q = d->qlist; q != 0; q = q->list)
        {
            if(q->nexttry == 0 || q->nexttry > d->now.tv_sec) continue;
            if(q->tries == 3)
            { // done retrying, expire and reset
                _c_expire(d,&d->cache[_namehash(q->name) % LPRIME]);
                _q_reset(d,q);
                continue;
            }
            ret++;
            q->nexttry = d->now.tv_sec + ++q->tries;
            if(nextbest == 0 || q->nexttry < nextbest)
                nextbest = q->nexttry;
            // if room, add all known good entries
            c = 0;
            while((c = _c_next(d,c,q->name,q->type)) != 0 && c->rr.ttl > d->now.tv_sec + 8 && message_packet_len(m) + _rr_len(&c->rr) < d->frame)
            {
                message_an(m,q->name,q->type,d->class,c->rr.ttl - d->now.tv_sec);
                _a_copy(m,&c->rr);
            }
        }
        d->checkqlist = nextbest;
    }

    if(d->now.tv_sec > d->expireall)
        _gc(d);

    return ret;
}

struct timeval *mdnsd_sleep(mdnsd d)
{
    int sec, usec;
    mdnsdr r;
    d->sleep.tv_sec = d->sleep.tv_usec = 0;
    #define RET while(d->sleep.tv_usec > 1000000) {d->sleep.tv_sec++;d->sleep.tv_usec -= 1000000;} return &d->sleep;

    // first check for any immediate items to handle
    if(d->uanswers || d->a_now) return &d->sleep;

    mdnsd_gettimeofday(&d->now,0);

    if(d->a_pause)
    { // then check for paused answers
        if((usec = _tvdiff(d->now,d->pause)) > 0) d->sleep.tv_usec = usec;
        RET;
    }

    if(d->probing)
    { // now check for probe retries
        if((usec = _tvdiff(d->now,d->probe)) > 0) d->sleep.tv_usec = usec;
        RET;
    }

    if(d->a_publish)
    { // now check for publish retries
        if((usec = _tvdiff(d->now,d->publish)) > 0) d->sleep.tv_usec = usec;
        RET;
    }

    if(d->checkqlist)
    { // also check for queries with known answer expiration/retry
        if((sec = d->checkqlist - d->now.tv_sec) > 0) d->sleep.tv_sec = sec;
        RET;
    }

    // last resort, next gc expiration
    if((sec = d->expireall - d->now.tv_sec) > 0) d->sleep.tv_sec = sec;
    RET;
}

void mdnsd_query(mdnsd d, char *host, int type, int (*answer)(mdnsda a, void *arg), void *arg)
{
    struct query *q;
    struct cached *cur = 0;
    int i = _namehash(host) % SPRIME;
    if(!(q = _q_next(d,0,host,type)))
    {
        if(!answer) return;
        q = (struct query *)malloc(sizeof(struct query));
        bzero(q,sizeof(struct query));
        q->name = strdup(host);
        q->type = type;
        q->next = d->queries[i];
        q->list = d->qlist;
        d->qlist = d->queries[i] = q;
        while(cur = _c_next(d,cur,q->name,q->type))
            cur->q = q; // any cached entries should be associated
        _q_reset(d,q);
        q->nexttry = d->checkqlist = d->now.tv_sec; // new questin, immediately send out
    }
    if(!answer)
    { // no answer means we don't care anymore
        _q_done(d,q);
        return;
    }
    q->answer = answer;
    q->arg = arg;
}

mdnsda mdnsd_list(mdnsd d, char *host, int type, mdnsda last)
{
    return (mdnsda)_c_next(d,(struct cached *)last,host,type);
}

mdnsdr mdnsd_shared(mdnsd d, char *host, int type, long int ttl)
{
    int i = _namehash(host) % SPRIME;
    mdnsdr r;
    r = (mdnsdr)malloc(sizeof(struct mdnsdr_struct));
    bzero(r,sizeof(struct mdnsdr_struct));
    r->rr.name = strdup(host);
    r->rr.type = type;
    r->rr.ttl = ttl;
    r->next = d->published[i];
    d->published[i] = r;
    return r;
}

mdnsdr mdnsd_unique(mdnsd d, char *host, int type, long int ttl, void (*conflict)(char *host, int type, void *arg), void *arg)
{
    mdnsdr r;
    r = mdnsd_shared(d,host,type,ttl);
    r->conflict = conflict;
    r->arg = arg;
    r->unique = 1;
    _r_push(&d->probing,r);
    d->probe.tv_sec = d->now.tv_sec;
    d->probe.tv_usec = d->now.tv_usec;
    return r;
}

void mdnsd_done(mdnsd d, mdnsdr r)
{
    mdnsdr cur;
    if(r->unique && r->unique < 5)
    { // probing yet, zap from that list first!
        if(d->probing == r) d->probing = r->list;
        else {
            for(cur=d->probing;cur->list != r;cur = cur->list);
            cur->list = r->list;
        }
        _r_done(d,r);
        return;
    }
    r->rr.ttl = 0;
    _r_send(d,r);
}

void mdnsd_set_raw(mdnsd d, mdnsdr r, char *data, int len)
{
    free(r->rr.rdata);
    r->rr.rdata = (unsigned char *)malloc(len);
    memcpy(r->rr.rdata,data,len);
    r->rr.rdlen = len;
    _r_publish(d,r);
}

void mdnsd_set_host(mdnsd d, mdnsdr r, char *name)
{
    free(r->rr.rdname);
    r->rr.rdname = strdup(name);
    _r_publish(d,r);
}

void mdnsd_set_ip(mdnsd d, mdnsdr r, unsigned long int ip)
{
    r->rr.ip = ip;
    _r_publish(d,r);
}

void mdnsd_set_srv(mdnsd d, mdnsdr r, int priority, int weight, int port, char *name)
{
    r->rr.srv.priority = priority;
    r->rr.srv.weight = weight;
    r->rr.srv.port = port;
    mdnsd_set_host(d,r,name);
}

