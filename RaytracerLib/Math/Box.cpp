#include "PCH.h"
#include "Box.h"


namespace rt {
namespace math {


Vector Box::GetVertex(int id) const
{
    Vector temp;
    temp.f[0] = (id & (1 << 0)) ? max.f[0] : min.f[0];
    temp.f[1] = (id & (1 << 1)) ? max.f[1] : min.f[1];
    temp.f[2] = (id & (1 << 2)) ? max.f[2] : min.f[2];
    return temp;
}

Vector Box::SupportVertex(const Vector& dir) const
{
    return Vector::SelectBySign(max, min, dir);
}

void Box::MakeFromPoints(const Vector* points, int number)
{
    if (number <= 0)
    {
        min = max = Vector();
    }
    else
    {
        min = max = points[0];

        for (int i = 1; i < number; i++)
        {
            min = Vector::Min(min, points[i]);
            max = Vector::Max(max, points[i]);
        }
    }
}


} // namespace math
} // namespace rt
