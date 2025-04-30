/**
 * Actual data for svg_icons.h
 */

namespace gui_icons {

const char* kEditPen = R"--(
<svg xmlns="http://www.w3.org/2000/svg" height="15.976562px" viewBox="0 0 16 15.976562" width="16px">
<path d="m 11 0.976562 l -2 2 h 1.5 c 0.132812 0 0.257812 0.050782 0.355469 0.144532 l 1.984375 1.988281
     c 0.09375 0.09375 0.144531 0.21875 0.144531 0.351563 v 1.589843 l 2.074219 -2.074219 v -1.941406
     l -2.058594 -2.058594 z m -3 3 l -7 7 v 4 h 4.058594 l 6.925781
     -6.925781 v -2.382812 l -1.691406 -1.691407 z m 0 0" fill="#222222"/>
</svg>
)--";

const char* kCheckmark = R"--(
<svg xmlns="http://www.w3.org/2000/svg" height="16px" viewBox="0 0 16 16" width="16px">
<path d="m 13.753906 4.667969 c 0.175782 -0.199219 0.261719 -0.460938
  0.246094 -0.722657 c -0.019531 -0.265624 -0.140625 -0.511718 -0.339844
  -0.6875 c -0.199218 -0.175781 -0.460937 -0.265624 -0.726562 -0.246093 c
  -0.265625 0.015625 -0.511719 0.140625 -0.6875 0.339843 l -6.296875
  7.195313 l -2.242188 -2.246094 c -0.390625 -0.390625 -1.023437 -0.390625
  -1.414062 0 c -0.1875 0.1875 -0.292969 0.445313 -0.292969 0.710938
  s 0.105469 0.519531 0.292969 0.707031 l 3 3 c 0.195312 0.195312 0.464843
  0.300781 0.742187 0.292969 c 0.277344 -0.011719 0.535156 -0.132813
  0.71875 -0.34375 z m 0 0" fill="#222222"/></svg>
)--";

const char* kExpandSvg = R"--(
<svg viewBox="0 0 16 16" xmlns="http://www.w3.org/2000/svg">
    <style
        type="text/css"
        id="current-color-scheme">
        .ColorScheme-Text {
            color:#232629;
        }
    </style>
    <path d="M2 4v1h12V4zm.707 3L2 7.707l6 6 6-6L13.293 7 8 12.293 2.707 7z"
     class="ColorScheme-Text" fill="currentColor"/>
</svg>
)--";

const char* kCollapseSvg = R"--(
<svg viewBox="0 0 16 16" xmlns="http://www.w3.org/2000/svg">
    <style
        type="text/css"
        id="current-color-scheme">
        .ColorScheme-Text {
            color:#232629;
        }
    </style>
    <path class="ColorScheme-Text"
         d="M2 4v1h12V4zm6 2.293l-6 6 .707.707L8 7.707 13.293 13l.707-.707z"
         fill="currentColor"/>
</svg>
)--";
}  // namespace gui_icons
