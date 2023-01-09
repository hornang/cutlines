#pragma once

#include <array>
#include <optional>
#include <vector>

#include "cutlines_export.h"

// Cohen-Sutherland line clipping code from:
// https://www.geeksforgeeks.org/line-clipping-set-1-cohen-sutherland-algorithm/

namespace cutlines {

using Point = std::array<double, 2>;
using Line = std::vector<Point>;

struct Rect
{
    double xMin;
    double xMax;
    double yMin;
    double yMax;
};

CUTLINES_EXPORT std::vector<Line> clip(const Line &line, const Rect &rect);

enum class Edge {
    NoEdge,
    Left,
    Right,
    Bottom,
    Top,
};

struct ClipResult
{
    Edge startClipEdge = Edge::NoEdge;
    Edge endClipEdge = Edge::NoEdge;
    Point start;
    Point end;
};

CUTLINES_EXPORT std::optional<ClipResult> CohenSutherlandLineClip(const Point &a,
                                                                  const Point &b,
                                                                  const Rect &rect);
}

CUTLINES_EXPORT std::ostream &operator<<(std::ostream &os, const std::pair<cutlines::Point, cutlines::Point> &lineSegment);
CUTLINES_EXPORT std::ostream &operator<<(std::ostream &os, const cutlines::Point &point);
CUTLINES_EXPORT std::ostream &operator<<(std::ostream &os, const cutlines::Line &line);
CUTLINES_EXPORT std::ostream &operator<<(std::ostream &os, const std::vector<cutlines::Line> &lines);
