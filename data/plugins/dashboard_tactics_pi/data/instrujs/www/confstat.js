/* $Id: confstat.js, v1.0 2019/11/30 VaderDarth Exp $
 * OpenCPN dashboard_tactics plug-in / WebKit based instruments
 * Licensed under MIT - see distribution.
 */
/* >>>> If you plan to modify this file, please make a backup first! <<<< */

/* Overriding the persistent configuration - useful if file:// does not save
   and http:// cannot be used or you want to absolutely override default values per
   each instrument. Otherwise do not define anything here. */

/* >>>> JavaScript syntax error(s) will prevent instruments to start <<<< */

/* If there is an error, take your browser, hit Shift+Ctrl+I for debugger and load
   the instrument HTLM-page (named index.html). Observe messages in the Console. */

const instrustatconf = {
    instruconf: [
        {
            uid: '3ba69928-d392-4583-8f9a-a323641063d7',
            conf: function () {
                return {
                    version    : 1,
                    path       : '',
                    title      : '',
                    symbol     : '',
                    unit       : '',
                    display    : '',
                    decimals   : 0,
                    minval     : 0,
                    loalert    : 0,
                    hialert    : 0,
                    maxval     : 0,
                    multiplier : 1,
                    divider    : 1,
                    offset     : 0,
                    dbfunc     : '',
                    dbnum      : 0,
                    wrnmsg     : false
                }
            }
        },
        {
            uid: '4abb6928-e311-5583-8f9b-c32d641da3e8',
            conf: function () {
                return {
                    version    : 1,
                    path       : '',
                    title      : '',
                    symbol     : '',
                    unit       : '',
                    display    : '',
                    decimals   : 0,
                    minval     : 0,
                    loalert    : 0,
                    hialert    : 0,
                    maxval     : 0,
                    multiplier : 1,
                    divider    : 1,
                    offset     : 0,
                    dbfunc     : '',
                    dbnum      : 0,
                    wrnmsg     : false
                }
            }
        }
    ],
    /* No changes below this line please */
    getObj : function( instruid ) {
        for ( var i = 0; i < this.instruconf.length; i++  ) {
            if ( instruid === this.instruconf[ parseInt(i) ].uid ) {
                return this.instruconf[ parseInt(i) ].conf()
            }
        }
        return null
    }
}
window.instrustatconf = instrustatconf
