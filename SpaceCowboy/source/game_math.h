#pragma once
#include <SDL.h>
#include <math.h>
#include <vector>
#include <map>
#include <iostream>

struct Rect {
    float x;
    float y;
    float w;
    float h;
    bool collision_aabb(Rect r2)
    {
        if (x < r2.x + r2.w &&
            x + w > r2.x &&
            y < r2.y + r2.h &&
            y + h > r2.y)
        {
            return true;
        }
        return false;
    }
};
struct Vector2
{
    float x;
    float y;
    Vector2(float X, float Y) :
        x(X), y(Y)
    {
    }
    Vector2()
    {
        Vector2(0, 0);
    }
    Vector2 normal()
    {
        return Vector2(y, -x);
    }

    float get_magnitude()
    {
        return (float)sqrt(pow(x,2) + pow(y,2));
    }
    float get_distance(Vector2 v2) {
        return (float)sqrt(pow(x-v2.x, 2) + pow(y-v2.y, 2));
    }
    void normalize()
    {
        float dist = get_magnitude();
        if (dist>0) {
        x = x / dist;
        y = y / dist;
        }
    }
    void add_vector(Vector2 v)
    {
        x = x + v.x;
        y = y + v.y;
    }
    void scale(float scalar)
    {
        x = x * scalar;
        y = y * scalar;
    }
    void clamp(float clamped_magnitude)
    {
        if (get_magnitude() > clamped_magnitude)
        {
            normalize();
            scale(clamped_magnitude);
        }
    }
    void move_towards(Vector2 target, float distance)
    {
        Vector2 interp;
        interp.x = target.x - x;
        interp.y = target.y - y;
        interp.normalize();
        interp.scale(distance);
        // Deal with weird floating point rounding errors by checking 0 crossing
        if (x > target.x && x + interp.x < target.x) x = target.x;
        else if (x < target.x && x + interp.x > target.x) x = target.x;
        else x = x + interp.x;
        if (y > target.y && y + interp.y < target.y) y = target.y;
        else if (y < target.y && y + interp.y > target.y) y = target.y;
        else y = y + interp.y;
    }

    Vector2 get_closest_cardinal() {
        Vector2 unit_vector;
        unit_vector.x = x;
        unit_vector.y = y;
        unit_vector.normalize();
        Vector2 V2_ZERO = { 0,0 };
        Vector2 V2_UP = { 0,-1 };
        Vector2 V2_DOWN = { 0,1 };
        Vector2 V2_LEFT = { -1, 0 };
        Vector2 V2_RIGHT = { 1, 0 };
        float up_dist = unit_vector.get_distance(V2_UP);
        float down_dist = unit_vector.get_distance(V2_DOWN);
        float left_dist = unit_vector.get_distance(V2_LEFT);
        float right_dist = unit_vector.get_distance(V2_RIGHT);

        float min_dist = up_dist;
        if (down_dist < min_dist) min_dist = down_dist;
        if (left_dist < min_dist) min_dist = left_dist;
        if (right_dist < min_dist) min_dist = right_dist;
        
        if (min_dist == right_dist) return V2_RIGHT;
        if (min_dist == left_dist) return V2_LEFT;
        if (min_dist == up_dist) return V2_UP;
        if (min_dist == down_dist) return V2_DOWN;
        return V2_ZERO;
    }

    
};

Vector2 operator + (Vector2 a, Vector2 b);
Vector2 operator - (Vector2 a, Vector2 b);
Vector2 operator * (float t, Vector2 a);

Vector2 lerp(Vector2 p0, Vector2 p1, float t);
float dot(Vector2 p0, Vector2 p1);
float cross(Vector2 p0, Vector2 p1);
bool is_point_in_triangle(Vector2 p, Vector2 a, Vector2 b, Vector2 c);


class PathComponent {
private:
    PathComponent* nextComponent;

public:
    float length;
    virtual Vector2 GetPointAtDistance(float d) = 0; // returns the vector if path is traveled by distance d
    virtual void UpdateLength()=0;
    void SetNextComponent(PathComponent* c)
    {
        nextComponent = c;
    }
    PathComponent* GetNextComponent()
    {
        return nextComponent;
    }
   virtual Vector2* GetLastPoint() = 0;
   virtual void PrettyPrint() = 0;
};




class Line : public PathComponent
{
    
public:
    Vector2* start;
    Vector2* end;
    Line(Vector2* s, Vector2* e)
    {
        start = s;
        end = e;
        UpdateLength();
    }
    void UpdateLength()
    {
        length = start->get_distance(*end);
    }
    
    Vector2 GetPointAtDistance(float d)
    {
        return lerp(*start, *end, d / length);
    }
    Vector2* GetLastPoint() {
        return end;
    }
    void PrettyPrint() {
        printf("Line ({%.4f,%.4f), (%.4f,%.4f)\n", start->x, start->y, end->x, end->y);
    }
};

class NBezier : public PathComponent
{
public:
    std::map<float, float> distance_table;
    std::vector<Vector2*> points;
    void PrettyPrint() {
        printf("NBezier()\n");
        for (Vector2* v : points)
        {
            printf("     ({%.4f,%.4f)\n", v->x, v->y);
        }
    }
    Vector2* GetLastPoint() {
        return points[points.size()-1];
    }
    Vector2* AddVector(Vector2 v)
    {
        Vector2* new_vector = new Vector2{ v.x, v.y };
        points.push_back(new_vector);
        return new_vector;
        
    }
    Vector2* AddVector(Vector2* new_vector)
    {
        points.push_back(new_vector);
        return new_vector;
    }
    std::vector<Vector2> GetPoint(float t, std::vector<Vector2> controlPoints, int order = 3)
    {
        // TODO: length of controlPoints must be order + 1
        std::vector<Vector2> segments;
        for (int n = 0; n < order; n++)
        {
            segments.push_back(lerp(controlPoints[n], controlPoints[n + 1], t));
        }
        if (order > 1) return GetPoint(t, segments, order - 1);
        else return segments;
    }
    void UpdateLength()
    {
        // default, approximate length with a resolution of 50 line segments
        UpdateLength(50);
    }
    void UpdateLength(int n)
    {
        // approximates the length of the curve by calculating the length of n line segments
        length = 0;
        float increment = 1.0f / n;
        Vector2 this_point = GetPoint(0);

        // create a lookup table at the same time that maps distance values to t values
        // do this at the same time as approximating the distance to kill 2 birds with one banana
        distance_table.clear();
        distance_table.insert(std::pair<float, float>(0, 0));
        for (float t = increment; t <= 1; t += increment)
        {
            Vector2 last_point = this_point;
            this_point = GetPoint(t);
            length += last_point.get_distance(this_point);

            // The accumulated distance at this t value
            // gets associated with the t in the table for mapping later
            // allows normalized travel over the arc length.
            
            distance_table.insert(std::pair<float, float>(length, t));
        }
    }
    Vector2 GetPoint(float t)
    {
        std::vector<Vector2> control_points;
        for (std::vector<Vector2*>::iterator i = points.begin(); i != points.end(); ++i)
             control_points.push_back(**i);
        std::vector<Vector2> p = GetPoint(t, control_points, control_points.size()-1);
        return p[0];
    }
    Vector2 GetPointAtDistance(float d)
    {
        float t0, t1, d0, d1, t;
        auto it = distance_table.upper_bound(d);
        
        t1 = it->second;
        d1 = it->first;
        it--;
        t0 = it->second;
        d0 = it->first;
        
        
        // interpolate between the two t values in the lookup table
        t = t0 + (t1 - t0) * ((d - d0) / (d1 - d0));

        return GetPoint(t);
    }

};

class Path {
private:
    PathComponent* root;
    PathComponent* end;
    bool loop = false;
    Vector2* default_build_point;
    
public:
    float length = 0;
    void SetFirstBuildPoint(Vector2 v)
    {
        default_build_point = new Vector2(v.x, v.y);
    }
    void SetRoot(PathComponent* r)
    {
        root = r;
        end = root;
        UpdateLengths();
    }
    void SetLoop(bool l)
    {
        loop = l;
    }
    PathComponent* GetRoot()
    {
        return root;
    }
    void AddComponent(PathComponent* p)
    {
        end->SetNextComponent(p);
        end = p;
        UpdateLengths();

    }
    Vector2* GetLastPoint()
    {
        PathComponent* node = root;
        if (!node) return default_build_point;
        while (node->GetNextComponent())
        {
            node = node->GetNextComponent();
        }
        return node->GetLastPoint();
    }
    void BuildLine(Vector2 e)
    {
        Vector2* start = GetLastPoint();
        Vector2* end = new Vector2;
        if (!start) {
            start = new Vector2;
            start->x = e.x - 50;
            start->y = e.y;
        }
        end->x = e.x;
        end->y = e.y;
        Line* line = new Line(start, end);
        if (!root) SetRoot(line);
        else AddComponent(line);
    }

    void BuildCurve(Vector2 e)
    {
        Vector2* v0 = GetLastPoint();
        Vector2* v1 = new Vector2;
        if (!v0) {
            v0 = new Vector2;
            v0->x = e.x - 50;
            v0->y = e.y;
        }
        v1->x = e.x;
        v1->y = e.y;
        NBezier* curve = new NBezier();
        curve->AddVector(v0);
        curve->AddVector(v1);
        if (!root) SetRoot(curve);
        else AddComponent(curve);
    }
    void AddBezierPoint(Vector2 e)
    {
        NBezier* curve = dynamic_cast<NBezier*>(end);
        if (curve)
        {
            Vector2* v0 = new Vector2;
            v0->x = e.x;
            v0->y = e.y;
            curve->AddVector(v0);
        }
    }
    Vector2 GetPointAtDistance(float d)
    {
        PathComponent* node = root;
        while (d > 0)
        {
            if (node->length > d)
            {
                return node->GetPointAtDistance(d);
            }
            d -= node->length;
            node = node->GetNextComponent();
            if (!node) {
                if (loop) node = root;
                else break;
            }
        }
        // at the end of the path, return the last vector of the path
        return end->GetPointAtDistance(end->length);
    }
    void UpdateLengths()
    {
        PathComponent* node = root;
        length = 0;
        if (!node) return;
        while (node)
        {
            node->UpdateLength();
            length += node->length;
            node = node->GetNextComponent();
        }
    }
    void PrettyPrint()
    {
        printf("Path Structure:\n");
        PathComponent* node = root;
        if (!node) return;
        while (node)
        {
            node->PrettyPrint();
            node = node->GetNextComponent();
        }
    }
};

struct Shape {
    virtual std::vector<Vector2> GetPoints()=0;
    Rect GetBoundingBox()
    {
        float minx = INFINITY, miny = INFINITY, maxx = -INFINITY, maxy = -INFINITY;
        for (auto& v : GetPoints())
        {
            if (v.x < minx) minx = v.x;
            if (v.y < miny) miny = v.y;
            if (v.x > maxx) maxx = v.x;
            if (v.y > maxy) maxy = v.y;
        }
        return { minx, miny, maxx - minx, maxy - miny };
    }
};
struct Triangle : Shape {
    Vector2 va;
    Vector2 vb;
    Vector2 vc;
    Triangle(Vector2 VA, Vector2 VB, Vector2 VC) :
        va(VA), vb(VB), vc(VC)
    {
    }
    std::vector<Vector2> GetPoints() {
        return { va, vb, vc };
    }

};

Triangle operator + (Triangle t, Vector2 a);

struct Polygon : Shape {
    std::vector<Vector2> vertices;
    int vertices_size()
    {
        return (int)vertices.size();
    }
    std::vector<Vector2> GetPoints() {
        return vertices;
    }
    std::vector<Vector2> triangulate()
    {
        std::vector<int> open;
        std::vector<int> closed;
        for (int i = 0; i < vertices_size(); i++)
        {
            open.push_back(i);
        }
        while (open.size() > 3)
        {
            for (int i = 0; i < open.size(); i++)
            {
                int bi = i - 1;
                if (bi < 0) bi += open.size();
                int ci = i + 1;
                if (ci >= open.size()) ci -= open.size();
                int a = open[i];
                int b = open[bi];
                int c = open[ci];

                Vector2 va = vertices[a];
                Vector2 vb = vertices[b];
                Vector2 vc = vertices[c];
                va.y = 0 - va.y;
                vb.y = 0 - vb.y;
                vc.y = 0 - vc.y;
                Vector2 va_to_vb = vb - va;
                Vector2 va_to_vc = vc - va;
                float cr = cross(va_to_vb, va_to_vc);
                if (cr < 0.0f)
                {
                    // reflex vertex, greater than 180 degrees
                    // not a valid ear
                    continue;
                }

                bool is_ear = true;
                // test to see if any vertices are within the ear
                for (int j = 0; j < vertices.size(); j++)
                {
                    if (j == a || j == b || j == c)
                    {
                        // dont test vertices that make up the ear, duh
                        continue;
                    }

                    Vector2 p = vertices[j];
                    p.y = 0 - p.y;
                    if (is_point_in_triangle(p, vb, va, vc)) {
                        // a point of the polygon falls within the triangle, so this isn't an ear
                        // stop testing
                        is_ear = false;
                        break;
                    }
                }
                if (is_ear)
                {
                    closed.push_back(b);
                    closed.push_back(a);
                    closed.push_back(c);
                    // remove vertex from open list
                    std::vector<int>::iterator it = open.begin() + i;
                    open.erase(it);
                    // start over at beginning
                    break;
                }
                  
            }
        }
        closed.push_back(open[0]);
        closed.push_back(open[1]);
        closed.push_back(open[2]);

        std::vector<Vector2> triangles;
        for (int i : closed)
        {
            triangles.push_back(vertices[i]);
        }
        return triangles;
    }
    std::vector<Triangle> Triangulate()
    {
        std::vector<Triangle> triangles;
        std::vector<Vector2> points = triangulate();
        for (int i = 0; i < points.size(); i += 3)
        {
            Vector2 a = points[i];
            Vector2 b = points[i + 1];
            Vector2 c = points[i + 2];
            triangles.push_back({ a,b,c });
        }
        return triangles;
    }
    void PrettyPrint()
    {
        printf("Polygon Structure:\n");
        for (Vector2 &v : vertices)
        {
            printf("   (%.3f, %.3f)\n", v.x, v.y);
        }
    }
};


bool triangle_overlap(Triangle& t1, Triangle& t2);
bool polygon_overlap(Shape& a, Shape& b);


// dynamics

class Dynamic
{
private:
    Polygon collision_polygon;
    std::vector<Triangle> collision_triangles;
public:
    Vector2 location;
    virtual void im_abstract_now() {} // needed this for dynamic downcasting...
    void SetLocation(Vector2 v)
    {
        location = v;
    }
    void SetCollisionPolygon(Polygon p)
    {
        collision_polygon = p;
        collision_triangles = collision_polygon.Triangulate();
    }
    Polygon* GetCollisionPolygon() {
        return &collision_polygon;
    }
    std::vector<Triangle> get_translated_triangles() {
        
        std::vector<Triangle> triangles;
        for (Triangle t : collision_triangles)
        {
            Triangle t2 = t;
            t2.va = t2.va + location;
            t2.vb = t2.vb + location;
            t2.vc = t2.vc + location;
            triangles.emplace_back(t2);
        }
        
        return triangles;
    }
    Polygon get_translated_polygon() {
        Polygon p_t;
        for (auto & v : collision_polygon.vertices)
        {
            p_t.vertices.push_back(v + location);
        }

        return p_t;
    }
    bool is_colliding_against(std::vector<Triangle>* triangles)
    {
        
        Polygon p = get_translated_polygon();
        for (Triangle t : *triangles)
        {
            // check bounding boxes first
            Rect r1 = p.GetBoundingBox();
            Rect r2 = t.GetBoundingBox();
            if (r1.collision_aabb(r2))
            {
                // bounding boxes overlap so it is worth it to check the more
                // processor intensive polygon_overlap that uses SAS
                if (polygon_overlap(p, t))
                    return true;
            }

            
        }
        
        return false;
    }

    bool is_colliding_against(Dynamic* d)
    {
        Polygon p1 = get_translated_polygon();
        Polygon p2 = d->get_translated_polygon();
        
        Rect r1 = p1.GetBoundingBox();
        Rect r2 = p2.GetBoundingBox();
        if (r1.collision_aabb(r2))
        {
            // bounding boxes overlap so it is worth it to check the more
            // processor intensive polygon_overlap that uses SAS
            if (polygon_overlap(p1, p2))
                return true;
        }

        return false;
    }

};

class MovementController
{
public:
    Dynamic* dynamic_obj;
    virtual bool Update(float delta_time) = 0;
    bool enabled = false;
    float travel_to = 0;
    float distance_traveled;
    float end_distance = 0;
    virtual bool IsTravelComplete() = 0;

    void SetTravelTo(float t)
    {
        travel_to = t;
        enabled = true;
    }
    virtual bool IsComplete() = 0;
    
};


class mcPathFollower : public MovementController
{
public:
    Path* path;
    float scalar_velocity;
    mcPathFollower(Dynamic* ob, Path* p, float svelocity)
    {
        dynamic_obj = ob;
        path = p;
        scalar_velocity = svelocity;
        distance_traveled = 0;
        end_distance = p->length;
    }
    bool IsTravelComplete() override
    {
        return distance_traveled >= travel_to;
    };

    bool IsComplete()
    {
        return distance_traveled >= end_distance;
    }
    bool Update(float delta_time) override
    {
        if (enabled && distance_traveled < travel_to)
        {
            distance_traveled += delta_time * scalar_velocity;
            dynamic_obj->location = path->GetPointAtDistance(distance_traveled);
        }
        return true;
    }
};

class mcLinear : public MovementController
{
public:
    Vector2 velocity;
    mcLinear(Dynamic* ob, Vector2 v, float length)
    {
        dynamic_obj = ob;
        velocity = v;
        distance_traveled = 0;
        end_distance = length;
    }
    bool IsTravelComplete() override
    {
        return distance_traveled >= travel_to;
    };
    bool IsComplete()
    {
        return distance_traveled >= end_distance;
    }
    bool Update(float delta_time) override
    {
        if (enabled && distance_traveled < travel_to)
        {
            dynamic_obj->location = dynamic_obj->location + (delta_time* velocity);
            distance_traveled += velocity.get_magnitude() * delta_time;
        }
        return true;
    }
};