#include <vector>
#include <exception>
#include <cstdlib>
#include <SFML/Graphics.hpp>
#include <cmath>
#include <iostream>

class Matrix {
    struct MatrixSize {
        int w;
        int h;
    } size;
    std::vector<double> vec;
public:
    Matrix(int h, int w, const double* coefficients) {
        size.w = w;
        size.h = h;
        vec.assign(coefficients, coefficients + w * h);
    }
    Matrix(int h, int w, std::initializer_list<double> coefficients) {
        size.w = w;
        size.h = h;
        vec.assign(coefficients.begin(), coefficients.end());
    }
    static Matrix I(int s, double a = 1.0) {
        double* c = new double[s * s];
        for (int y = 0; y < s; y++) {
            for (int x = 0; x < s; x++) {
                c[y * s + x] = ((x == y) ? a : 0.0);
            }
        }
        Matrix i(s, s, c);
        delete[] c;
        return i;
    }
    static Matrix v3m4x1(sf::Vector3f vec) {
        double m[] = { vec.x,vec.y,vec.z,1 };
        return Matrix(4, 1, m);
    }
    sf::Vector3f transform(sf::Vector3f vec) {
        Matrix columnVec = Matrix::v3m4x1(vec);
        Matrix transformed = (*this) * columnVec;
        transformed = I(4, 1.0 / transformed.get(3, 0)) * transformed;
        return transformed.toVec();
    }
    sf::Vector2f project(sf::Vector3f vec) {
        Matrix columnVec = Matrix::v3m4x1(vec);
        Matrix projected = (*this) * columnVec;
        projected = I(4, 1.0 / projected.get(3, 0)) * projected;
        return projected.toVec2();
    }
    double get(int i, int j) const {
        return vec[size.w * i + j];
    }
    sf::Vector2f toVec2() const {
        return sf::Vector2f{ (float)get(0,0),(float)get(1,0) };
    }
    sf::Vector3f toVec()const {
        return sf::Vector3f((float)get(0, 0), (float)get(1, 0), (float)get(2, 0));
    }
    Matrix operator*(const Matrix& b) const {
        const Matrix& a = *this;
        if (a.size.w != b.size.h)
            throw std::runtime_error("Can't multiply matricies with incompatible sizes!");
        int h = a.size.h;
        int w = b.size.w;
        std::vector<double> v;
        for (int i = 0; i < h; i++)
            for (int j = 0; j < w; j++) {
                double sum = 0;
                for (int k = 0; k < a.size.w; k++)
                    sum += a.get(i, k) * b.get(k, j);
                v.push_back(sum);
            }
        return Matrix(h, w, v.data());
    }
    Matrix operator+(const Matrix& other)const {
        if (other.size.w != size.w || other.size.h != size.h) {
            throw std::runtime_error("Can't add matricies with incompatible sizes!");
        }
        std::vector<double> c(size.w * size.h);
        for (int i = 0; i < size.h; i++)
            for (int j = 0; j < size.w; j++)
                c[i * size.w + j] = get(i, j) + other.get(i, j);
        return Matrix(size.h, size.w, c.data());
    }
    static Matrix translate(double x, double y, double z) {
        return Matrix(4, 4, { 1,0,0,x,
                           0,1,0,y,
                           0,0,1,z,
                           0,0,0,1 });
    }
    static Matrix rotateZ(double phi) {
        return Matrix(4, 4, { std::cos(phi),-std::sin(phi),0,0,
                           std::sin(phi), std::cos(phi),0,0,
                           0            , 0            ,1,0,
                           0            , 0            ,0,1 });
    }
    static Matrix rotateY(double phi) {
        return Matrix(4, 4, { std::cos(phi),0,std::sin(phi),0,
                           0,            1,0,            0,
                          -std::sin(phi),0,std::cos(phi),0,
                           0,            0,0,            1 });
    }
    static Matrix rotateX(double phi) {
        return Matrix(4, 4, { 1,0,        0,
                           0,cos(phi),-sin(phi),
                           0,sin(phi), cos(phi) });
    }
    static Matrix scale(double x, double y, double z) {
        return Matrix(4, 4, { x,0,0,0,
                           0,y,0,0,
                           0,0,z,0,
                           0,0,0,1 });
    }


};
sf::Vector2f centerCoordinates(sf::Vector2f vec, const sf::RenderWindow& win) {
    auto size = win.getSize();
    return sf::Vector2f{ vec.x + size.x / 2,-vec.y + size.y / 2 };
}
int mod(int i, int n) {
    return (n + (i % n)) % n;
}
struct Shape3d {
    std::vector<sf::Vector3f> vertices;
    std::vector<std::pair<int, int> > edges;
    Shape3d& transform(Matrix transformation) {
        std::vector<sf::Vector3f> nVertices;
        for (const auto& v : vertices) {
            nVertices.push_back(transformation.transform(v));
        }
        vertices = nVertices;
        return *this;
    }

    void draw(sf::RenderWindow& win, Matrix projectionMatrix = Matrix::I(4)) {
        sf::VertexArray array;
        array.setPrimitiveType(sf::Lines);
        for (const auto& e : edges) {
            sf::Vector2f a = centerCoordinates(projectionMatrix.project(vertices[e.first]), win);
            sf::Vector2f b = centerCoordinates(projectionMatrix.project(vertices[e.second]), win);
            array.append(sf::Vertex(a, sf::Color::White));
            array.append(sf::Vertex(b, sf::Color::White));
        }
        win.draw(array);
    }

    static Shape3d cube() {
        return Shape3d{
            {
                {-1,-1,-1},
                {-1,-1,1},
                {-1,1,-1},
                {-1,1,1},
                {1,-1,-1},
                {1,-1,1},
                {1,1,-1},
                {1,1,1}
            },
            {
                {0,1},
                {0,2},
                {0,4},
                {1,3},
                {1,5},
                {2,3},
                {2,6},
                {3,7},
                {4,5},
                {4,6},
                {5,7},
                {7,6}
            }
        };
    }

};
const Matrix ISOMETRIC = Matrix(4, 4, { sqrt(3),0,-sqrt(3),      0,
                                          1, 2, 1,            0,
                                    sqrt(2),-sqrt(2),sqrt(2), 0,
                                           0,0,0,             sqrt(6) });
const double d = 400.0;
const Matrix PERSPECTIVE = Matrix(4, 4, { 1,0,0,  0,
                                       0,1,0,  0,
                                       0,0,1,  d,
                                       0,0,1 / d,1 });
int main() {
    Shape3d s = Shape3d::cube().transform(Matrix::scale(50, 50, 50));

    Matrix projection = PERSPECTIVE;

    sf::RenderWindow win(sf::VideoMode(800, 600), "CG");

    while (win.isOpen()) {
        win.clear();
        sf::Event event;

        while (win.pollEvent(event)) {
            if (event.type == sf::Event::Closed) {
                win.close();
            }
            if (event.type == sf::Event::KeyPressed) {
                switch (event.key.code)
                {
                case sf::Keyboard::Down:
                    s.transform(Matrix::rotateZ(3.1415 / 16.0));
                    break;
                case sf::Keyboard::Up:
                    s.transform(Matrix::rotateZ(-3.1415 / 16.0));
                    break;
                case sf::Keyboard::Left:
                    s.transform(Matrix::rotateY(3.1415 / 16.0));
                    break;
                case sf::Keyboard::Right:
                    s.transform(Matrix::rotateY(-3.1415 / 16.0));
                    break;
                case sf::Keyboard::W:
                    s.transform(Matrix::translate(0, 10, 0));
                    break;
                case sf::Keyboard::S:
                    s.transform(Matrix::translate(0, -10, 0));
                    break;
                case sf::Keyboard::A:
                    s.transform(Matrix::translate(-10, 0, 0));
                    break;
                case sf::Keyboard::D:
                    s.transform(Matrix::translate(10, 0, 0));
                    break;
                case sf::Keyboard::Q:
                    s.transform(Matrix::translate(0, 0, -10));
                    break;
                case sf::Keyboard::E:
                    s.transform(Matrix::translate(0, 0, 10));
                    break;
                case sf::Keyboard::I:
                    projection = ISOMETRIC;
                    break;
                case sf::Keyboard::P:
                    projection = PERSPECTIVE;
                    break;

                default:
                    break;
                }
            }

        }
        s.draw(win, projection);
        win.display();
    }
}