#include <cassert>
#include <cmath>
#include <iostream>
#include <numeric>
#include <optional>
#include <sstream>

#include "cutlines/cutlines.h"

typedef int OutCode;

using namespace cutlines;

const int INSIDE = 0; // 0000
const int LEFT = 1; // 0001
const int RIGHT = 2; // 0010
const int BOTTOM = 4; // 0100
const int TOP = 8; // 0100

// Compute the bit code for a point (x, y) using the clip rectangle
// bounded diagonally by (xmin, ymin), and (xmax, ymax)

OutCode ComputeOutCode(double x, double y, const cutlines::Rect &rect)
{
    const double &xmin = rect.xMin;
    const double &xmax = rect.xMax;
    const double &ymin = rect.yMin;
    const double &ymax = rect.yMax;

    OutCode code = INSIDE; // initialised as being inside of clip window

    if (x < xmin) // to the left of clip window
        code |= LEFT;
    else if (x > xmax) // to the right of clip window
        code |= RIGHT;
    if (y < ymin) // below the clip window
        code |= BOTTOM;
    else if (y > ymax) // above the clip window
        code |= TOP;

    return code;
}

static cutlines::Edge toEdge(OutCode code)
{
    switch (code) {
    case LEFT:
        return Edge::Left;
    case RIGHT:
        return Edge::Right;
    case BOTTOM:
        return Edge::Bottom;
    case TOP:
        return Edge::Top;
    default:
        return Edge::NoEdge;
    }
}

// Cohen–Sutherland clipping algorithm clips a line from
// P0 = (x0, y0) to P1 = (x1, y1) against a rectangle with
// diagonal from (xmin, ymin) to (xmax, ymax).
std::optional<cutlines::ClipResult> cutlines::CohenSutherlandLineClip(const Point &a, const Point &b, const Rect &rect)
{
    double x0 = a[0];
    double y0 = a[1];
    double x1 = b[0];
    double y1 = b[1];

    const double &xmin = rect.xMin;
    const double &xmax = rect.xMax;
    const double &ymin = rect.yMin;
    const double &ymax = rect.yMax;

    // compute outcodes for P0, P1, and whatever point lies outside the clip rectangle
    OutCode outcode0 = ComputeOutCode(x0, y0, rect);
    OutCode outcode1 = ComputeOutCode(x1, y1, rect);

    OutCode lastEdge0 = outcode0;
    OutCode lastEdge1 = outcode1;

    while (true) {
        if (!(outcode0 | outcode1)) {
            // bitwise OR is 0: both points inside window; trivially accept and exit loop

            ClipResult clipResult;
            clipResult.startClipEdge = toEdge(lastEdge0);
            clipResult.endClipEdge = toEdge(lastEdge1);
            clipResult.start = Point { x0, y0 };
            clipResult.end = Point { x1, y1 };
            return clipResult;
        } else if (outcode0 & outcode1) {
            // bitwise AND is not 0: both points share an outside zone (LEFT, RIGHT, TOP,
            // or BOTTOM), so both must be outside window; exit loop (accept is false)
            break;
        } else {
            // failed both tests, so calculate the line segment to clip
            // from an outside point to an intersection with clip edge
            double x, y;

            // At least one endpoint is outside the clip rectangle; pick it.
            OutCode outcodeOut = outcode1 > outcode0 ? outcode1 : outcode0;

            // Now find the intersection point;
            // use formulas:
            //   slope = (y1 - y0) / (x1 - x0)
            //   x = x0 + (1 / slope) * (ym - y0), where ym is ymin or ymax
            //   y = y0 + slope * (xm - x0), where xm is xmin or xmax
            // No need to worry about divide-by-zero because, in each case, the
            // outcode bit being tested guarantees the denominator is non-zero
            if (outcodeOut & TOP) { // point is above the clip window
                x = x0 + (x1 - x0) * (ymax - y0) / (y1 - y0);
                y = ymax;
            } else if (outcodeOut & BOTTOM) { // point is below the clip window
                x = x0 + (x1 - x0) * (ymin - y0) / (y1 - y0);
                y = ymin;
            } else if (outcodeOut & RIGHT) { // point is to the right of clip window
                y = y0 + (y1 - y0) * (xmax - x0) / (x1 - x0);
                x = xmax;
            } else if (outcodeOut & LEFT) { // point is to the left of clip window
                y = y0 + (y1 - y0) * (xmin - x0) / (x1 - x0);
                x = xmin;
            }

            // Now we move outside point to intersection point to clip
            // and get ready for next pass.
            if (outcodeOut == outcode0) {
                x0 = x;
                y0 = y;
                outcode0 = ComputeOutCode(x0, y0, rect);
                if (outcode0 != 0) {
                    lastEdge0 = outcode0;
                }
            } else {
                x1 = x;
                y1 = y;
                outcode1 = ComputeOutCode(x1, y1, rect);
                if (outcode1 != 0) {
                    lastEdge1 = outcode1;
                }
            }
        }
    }

    return {};
}

std::vector<cutlines::Line> cutlines::clip(const Line &line, const Rect &rect)
{
    if (line.size() < 2) {
        return {};
    }

    std::vector<Line> clippedLines;
    Line subLine;

    for (int i = 1; i < line.size(); i++) {

        std::optional<cutlines::ClipResult> clipped = CohenSutherlandLineClip(line[i - 1], line[i], rect);
        if (clipped.has_value()) {
            const cutlines::ClipResult &clipResult = clipped.value();
            if (subLine.empty()) {
                subLine.push_back(clipResult.start);
                subLine.push_back(clipResult.end);
                if (clipResult.endClipEdge != Edge::NoEdge) {
                    clippedLines.push_back(subLine);
                    subLine.clear();
                }
            } else {
                subLine.push_back(clipResult.end);
            }
        } else {
            if (!subLine.empty()) {
                clippedLines.push_back(subLine);
                subLine.clear();
            }
        }
    }

    if (!subLine.empty()) {
        clippedLines.push_back(subLine);
    }

    return clippedLines;
}

std::ostream &operator<<(std::ostream &os, const std::pair<cutlines::Point, cutlines::Point> &lineSegment)
{
    os << "[ " << lineSegment.first << " " << lineSegment.second << " ]";
    return os;
}

std::ostream &operator<<(std::ostream &os, const std::vector<Line> &lines)
{
    os << "[ ";
    for (const Line &line : lines) {
        os << line << " ";
    }
    os << "]";
    return os;
}

std::ostream &operator<<(std::ostream &os, const Line &line)
{
    os << "[ ";
    for (const Point &point : line) {
        os << point << " ";
    }
    os << "]";
    return os;
}

std::ostream &operator<<(std::ostream &os, const Point &point)
{
    os << "(" << point[0] << ", " << point[1] << ")";
    return os;
}
