#include <algorithm>
#include "game_math.h"

Vector2 operator + (Vector2 a, Vector2 b)
{
    return Vector2{ a.x + b.x, a.y + b.y };
}
Vector2 operator - (Vector2 a, Vector2 b)
{
    return Vector2{ a.x - b.x, a.y - b.y };
}
Vector2 operator * (float t, Vector2 a)
{
    return Vector2{t * a.x, t * a.y};
}

Triangle operator + (Triangle t, Vector2 a) {
    return Triangle(t.va + a, t.vb + a, t.vc + a);
}
Vector2 lerp(Vector2 p0, Vector2 p1, float t)
{
    // finds the linear interpolation between two points
    // at point t (0 to 1)
    
    return p0 + t * (p1 - p0);
}

float dot(Vector2 p0, Vector2 p1)
{
    return p0.x * p1.x + p0.y * p1.y;
}
float cross(Vector2 a, Vector2 b)
{
    return a.x*b.y - a.y * b.x;
}

bool is_point_in_triangle(Vector2 p, Vector2 a, Vector2 b, Vector2 c)
{
    // returns true if the point is within the given triangle
    Vector2 ab = b - a;
    Vector2 bc = c - b;
    Vector2 ca = a - c;

    Vector2 ap = p - a;
    Vector2 bp = p - b;
    Vector2 cp = p - c;

    float cross1 = cross(ab, ap);
    float cross2 = cross(bc, bp);
    float cross3 = cross(ca, cp);

    if (cross1 > 0 || cross2 > 0 || cross3 > 0) return false;
    return true;
}


bool polygon_overlap(Shape& a, Shape& b)
{
    // uses SAT algorithm
    Shape* shape1 = &a;
    Shape* shape2 = &b;
    float overlap = INFINITY;
    for (int shape = 0; shape < 2; shape++)
    {
        if (shape == 1)
        {
            // swap sides
            shape1 = &b;
            shape2 = &a;
        }

        std::vector<Vector2> shape1_points = shape1->GetPoints();
        std::vector<Vector2> shape2_points = shape2->GetPoints();

        std::vector<Vector2> shape1_edges;
        std::vector<Vector2> shape2_edges;

        for (int i = 0; i < shape1_points.size(); ++i)
        {
            Vector2 v0 = shape1_points[i];
            Vector2 v1 = shape1_points[(i + 1) % shape1_points.size()];
            Vector2 edge = v1 - v0;
            shape1_edges.push_back(edge);
        }
        
        for (int i = 0; i < shape2_points.size(); ++i)
        {
            Vector2 v0 = shape2_points[i];
            Vector2 v1 = shape2_points[(i + 1) % shape2_points.size()];
            Vector2 edge = v1 - v0;
            shape2_edges.push_back(edge);
        }
        
               
        for (auto edge : shape1_edges)
        {
            Vector2 projected_axis = edge.normal();
            // project points onto axis and calculate min and max
            // 1d points
            float min_r1 = INFINITY, max_r1 = -INFINITY;
            for (auto point1 : shape1_points)
            {
                float q = dot(point1, projected_axis);
                min_r1 = std::min(min_r1, q);
                max_r1 = std::max(max_r1, q);
            }

            float min_r2 = INFINITY, max_r2 = -INFINITY;

            for (Vector2 point2 : shape2_points)
            {
                float q = dot(point2, projected_axis);

                min_r2 = std::min(min_r2, q);
                max_r2 = std::max(max_r2, q);
            }
            overlap = std::min(std::min(max_r1, max_r2) - std::max(min_r1, min_r2), overlap);
            if (!(max_r2 >= min_r1 && max_r1 >= min_r2))
                return false;
        }
    }
    return true;
}



bool triangle_overlap(Triangle& t1, Triangle& t2) 
{
    // uses SAT algorithm
    Triangle* shape1 = &t1;
    Triangle* shape2 = &t2;
    float overlap = INFINITY;
    for (int shape = 0; shape < 2; shape++)
    {
        if (shape == 1)
        {
            // swap sides
            shape1 = &t2;
            shape2 = &t1;
        }
        std::vector<Vector2> shape1_edges;
        shape1_edges.push_back(shape1->vb - shape1->va);
        shape1_edges.push_back(shape1->vc - shape1->vb);
        shape1_edges.push_back(shape1->va - shape1->vc);
        std::vector<Vector2> shape2_edges;
        shape2_edges.push_back(shape2->vb - shape2->va);
        shape2_edges.push_back(shape2->vc - shape2->vb);
        shape2_edges.push_back(shape2->va - shape2->vc);

        std::vector<Vector2> shape1_points;
        shape1_points.push_back(shape1->va);
        shape1_points.push_back(shape1->vb);
        shape1_points.push_back(shape1->vc);
        std::vector<Vector2> shape2_points;
        shape2_points.push_back(shape2->va);
        shape2_points.push_back(shape2->vb);
        shape2_points.push_back(shape2->vc);
        for (auto edge : shape1_edges)
        {
            Vector2 projected_axis = edge.normal();
            // project points onto axis and calculate min and max
            // 1d points
            float min_r1 = INFINITY, max_r1 = -INFINITY;
            for (auto point1 : shape1_points)
            {
                float q = dot(point1, projected_axis);
                min_r1 = std::min(min_r1, q);
                max_r1 = std::max(max_r1, q);
            }

            float min_r2 = INFINITY, max_r2 = -INFINITY;
            
            for (Vector2 point2 : shape2_points)
            {
                float q = dot(point2, projected_axis);
                
                min_r2 = std::min(min_r2, q);
                max_r2 = std::max(max_r2, q);
            }
            overlap = std::min(std::min(max_r1, max_r2) - std::max(min_r1, min_r2), overlap);
            if (!(max_r2 >= min_r1 && max_r1 >= min_r2))
                return false;
        }
    }
    return true;
}