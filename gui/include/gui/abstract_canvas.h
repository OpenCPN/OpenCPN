#ifndef UnDo_HanDler__
#define UnDo_HanDler__

#include <model/route.h>

/**
 * @interface
 *
 * Handle higher levels of Undo actions
 */
class AbstractCanvas {
public:
  virtual void UndoAppendWaypoint(Route* route, bool no_route_left_to_redo) = 0;
  virtual void RedoAppendWaypoint(Route* route) = 0;
  virtual int GetRouteState() const = 0;
};
#endif  // UnDo_HanDler__
