#include <emscripten/bind.h>
#include <emscripten.h>
#include <iostream>
#include <vector>
#include <map>
#include <cstdlib>
#include <queue>
#include <set>
#include <ctime>
#include <sstream>
//#include <thread>
#include <fstream>
#include <algorithm>
#include <cmath>
using namespace std;
using namespace emscripten;


int M;





// This means M by M by M cube
#define FN 9
#define N (M + 2)
#define mp make_pair
#define rep(i, l, r) for (int i = l; i < r; i++)
#define rep_(i, l, r) for (i = l; i < r; i++)

int start;
stringstream f; //ofstream f;

char colors[6] = {'w', 'b', 'r', 'o', 'g', 'y'};

inline int sqr(int x) { return x * x; }

struct cood {
    int x, y, z;

    cood() {}

    cood(int x, int y, int z) : x(x), y(y), z(z) {}

    double dis() const {
        return sqr(2 * x - N + 1) + sqr(2 * y - N + 1) + sqr(2 * z - N + 1);
    }

    bool operator<(cood const &c) const {
        return dis() < c.dis() || dis() == c.dis() && mp(mp(x, y), z) < mp(mp(c.x, c.y), c.z);
    }

    int &operator[](int i) {
        return i == 0 ? x : (i == 1 ? y : z);
    }

    int operator[](int i) const {
        return i == 0 ? x : (i == 1 ? y : z);
    }

};

struct state {
    char a[FN][FN][FN];

    state() {
        rep(i, 0, N) rep(j, 0, N) rep(k, 0, N) a[i][j][k] = 0;
    }

    bool operator<(const state &s) const {
        rep(i, 0, N) rep(j, 0, N) rep(k, 0, N) {
                    if (a[i][j][k] < s.a[i][j][k]) return true;
                    if (a[i][j][k] > s.a[i][j][k]) return false;
                }
        return false;
    }

    bool operator==(const state &s) const {
        rep(i, 0, N) rep(j, 0, N) rep(k, 0, N) {
                    if (a[i][j][k] != s.a[i][j][k]) return false;
                }
        return true;
    }

    char operator[](cood c) const {
        return a[c[0]][c[1]][c[2]];
    }

    char &operator[](cood c) {
        return a[c[0]][c[1]][c[2]];
    }

} State;


pair<int, int> cood_2D(cood c) {
    int x, y;
    rep(i, 0, 3) {
        if (c[i] == 0) {
            x = c[(i + 1) % 3];
            y = c[(i + 2) % 3];
        }
        if (c[i] == N - 1) {
            x = c[(i + 2) % 3];
            y = c[(i + 1) % 3];
        }
    }
    while (!(2 * x == N - 1 && 2 * y == N - 1) && (2 * x > N - 1 || 2 * y >= N - 1)) {
        int tmp = y;
        y = x;
        x = N - 1 - tmp;
    }
    return mp(x, y);
}


typedef vector<string> alg;



void print_(alg a) {
    rep(i, 0, a.size()) cout << a[i] << " ";
    cout << endl;
}

struct bar {
    int st[3], ed[3];

    bool in(cood c) const {
        rep(i, 0, 3) if (c[i] < st[i] || c[i] >= ed[i]) return false;
        return true;
    }

    int get1d() const {
        rep(k, 0, 3) if (st[k] + 1 == ed[k] && st[k] >= 2 && st[k] < N - 2) return abs(2 * st[k] - N + 1);
    }

    bool operator<(bar const &b) const {
        return get1d() < b.get1d();
    }

    bool intersect(bar b) const {
        rep(i, 0, 3) if (st[i] >= b.ed[i] || ed[i] <= b.st[i]) return false;
        return true;
    }
};

typedef pair<cood, cood> edge;

struct perm {
    vector<pair<cood, cood> > p;
};

map<string, perm> perms;
vector<perm> permsP;
vector<string> permsS, laterTurnSet, edgeSet, edgeSet2, pureU, simpleSet, lazySet1, lazySet2;
vector<bar> bars;
vector<bar> bars1d[FN];
vector<cood> c2ds[FN][FN];
map<edge, int> mapEdgeC1d;
vector<edge> edges1d[FN];
vector<edge> seudoEdges;
vector<pair<char, char> > colorPairs;

alg inverse(alg a) {
    alg ret;
    for (int i = a.size() - 1; i >= 0; i--) {
        if (a[i][a[i].length() - 1] == '2' || a[i] == "flip") ret.emplace_back(a[i]);
        else if (a[i][a[i].length() - 1] == '\'') ret.emplace_back(a[i].substr(0, a[i].length() - 1));
        else ret.emplace_back(a[i] + '\'');
    }
    return ret;
}

alg readAlg(string file) {
    ifstream f(file);
    alg a;
    string x;
    while (f >> x) {
        a.push_back(x);
    }
//    a = inverse(a);
    return a;
}

alg algFromStr(string s) {
    stringstream f(s);
    alg a;
    string x;
    while (f >> x) {
        a.push_back(x);
    }
//    a = inverse(a);
    return a;
}

alg lazyA{"F", "R", "U", "R'", "U'", "F'"}, lazyAPrime = inverse(lazyA);

alg lazyB{"R'", "U2", "R", "U", "R'", "U", "R"}, lazyBPrime = inverse(lazyB);

alg lazyC{"R", "U", "R'", "U'", "R'", "F", "R2", "U'", "R'", "U'", "R", "U", "R'", "F'"};

alg lazyD{"R", "U'", "R", "U", "R", "U", "R", "U'", "R'", "U'", "R2"}, lazyDPrime = inverse(lazyD);

alg parityA, parityB;

void outText(string s) {
    string cmd = "document.getElementById(\"2333\").value = document.getElementById(\"2333\").value + `" + s + "`";
//    cmd = "alert(`" + f.str() + "`)";
    cout << cmd << endl;
    emscripten_run_script(cmd.c_str());
}


void print(alg a, stringstream &s) {
    rep(i, 0, a.size()) {
        if (a[i].substr(0, 7) == "insertA") {
            s << "R' U" << a[i].substr(7, a[i].size() - 7) << " R ";
            outText("R' U" + a[i].substr(7, a[i].size() - 7) + " R ");
        } else if (a[i].substr(0, 7) == "insertB") {
            s << "R U" << a[i].substr(7, a[i].size() - 7) << " R' ";
            outText("R U" + a[i].substr(7, a[i].size() - 7) + " R' ");
        } else if (a[i] == "flip") {
            s << "R U R' F R' F' R ";
            outText("R U R' F R' F' R ");
        } else if (a[i] == "lazyA") print(lazyA, s);
        else if (a[i] == "lazyB") print(lazyB, s);
        else if (a[i] == "lazyC") print(lazyC, s);
        else if (a[i] == "lazyD") print(lazyD, s);
        else if (a[i] == "parityA") print(parityA, s);
        else if (a[i] == "parityB") print(parityB, s);
        else if (a[i] == "lazyA'") print(lazyAPrime, s);
        else if (a[i] == "lazyB'") print(lazyBPrime, s);
        else if (a[i] == "lazyC'") print(lazyC, s);
        else if (a[i] == "lazyD'") print(lazyDPrime, s);
        else if (a[i] == "parityA'") print(parityA, s);
        else if (a[i] == "parityB'") print(parityB, s);
        else {
            s << a[i] << " ";
            outText(a[i] + " ");
        }
    }
    s << endl;
    outText("\n");

}


bool surface(cood c) {
    bool x = c.x == 0 || c.x == N - 1,
            y = c.y == 0 || c.y == N - 1,
            z = c.z == 0 || c.z == N - 1;
    return x + y + z == 1;
}

bool center(cood c) {
    bool x = c.x == 0 || c.x == N - 1,
            y = c.y == 0 || c.y == N - 1,
            z = c.z == 0 || c.z == N - 1,
            cx = c.x >= 2 && c.x <= N - 3,
            cy = c.y >= 2 && c.y <= N - 3,
            cz = c.z >= 2 && c.z <= N - 3;
    return x && cy && cz || cx && y && cz || cx && cy && z;
}


perm composite(perm p1, perm p2) {
    map<cood, cood> m;
    perm p;
    rep(i, 0, p1.p.size()) {
        m[p1.p[i].first] = p1.p[i].second;
    }
    rep(i, 0, p2.p.size()) {
        if (m.count(p2.p[i].second)) {
            p.p.emplace_back(mp(p2.p[i].first, m[p2.p[i].second]));
            m.erase(p2.p[i].second);
        } else p.p.emplace_back(mp(p2.p[i].first, p2.p[i].second));
    }
    for (const auto &item : m) {
        p.p.emplace_back(mp(item.first, item.second));
    }
    return p;
}

perm xRotate(int l, int r) {
    perm p;
    rep(i, l, r) rep(j, 0, N) rep(k, 0, N) {
                if (surface(cood(i, j, k))) {
                    p.p.emplace_back(mp(cood(i, j, k), cood(i, N - 1 - k, j)));
                }
            }
    return p;
}

perm yRotate(int l, int r) {
    perm p;
    rep(i, 0, N) rep(j, l, r) rep(k, 0, N) {
                if (surface(cood(i, j, k))) {
                    p.p.emplace_back(mp(cood(i, j, k), cood(N - 1 - k, j, i)));
                }
            }
    return p;
}

perm zRotate(int l, int r) {
    perm p;
    rep(i, 0, N) rep(j, 0, N) rep(k, l, r) {
                if (surface(cood(i, j, k))) {
                    p.p.emplace_back(mp(cood(i, j, k), cood(N - 1 - j, i, k)));
                }
            }
    return p;
}


void appoint(perm p, string one, string two, string prime) {
    perms[one] = p;
    perms[two] = composite(p, p);
    perms[prime] = composite(perms[two], p);
}

void render(int il, int ir, int jl, int jr, int kl, int kr, char c) {
    rep(i, il, ir) rep(j, jl, jr) rep(k, kl, kr) State.a[i][j][k] = c;
}

alg randomAlg() {
    int l = 25 + 20 * (N - 5);
    vector<string> turns = {"R", "U", "F", "L", "D", "B"};
    if (N >= 6) {
        turns.push_back("r");
        turns.push_back("u");
        turns.push_back("f");
    }
    if (N >= 7) {
        turns.push_back("l");
        turns.push_back("d");
        turns.push_back("b");
    }
    if (N >= 8) {
        turns.push_back("3r");
        turns.push_back("3u");
        turns.push_back("3f");
    }
    if (N >= 9) {
        turns.push_back("3l");
        turns.push_back("3d");
        turns.push_back("3b");
    }
    alg ret;
    rep(i, 0, l) {
        string tmp = turns[rand() % turns.size()];
        int rnd = rand() % 3;
        if (rnd == 0) ret.emplace_back(tmp + '\'');
        else if (rnd == 1) ret.emplace_back(tmp + '2');
        else ret.emplace_back(tmp);
    }
    return ret;
}

void cross(int a[3], int b[3], int res[3]) {
    res[0] = a[1] * b[2] - a[2] * b[1];
    res[1] = a[2] * b[0] - a[0] * b[2];
    res[2] = a[0] * b[1] - a[1] * b[0];
}

perm permFromStr(vector<string> s) {
    perm ret;
    rep(i, 0, s.size()) {
        ret = composite(ret, perms[s[i]]);
    }
    return ret;
}




void init() {
    State = state();
    perms.clear();
    permsP.clear();
    permsS.clear();
    laterTurnSet.clear();
    edgeSet.clear();
    edgeSet2.clear();
    pureU.clear();
    simpleSet.clear();
    lazySet1.clear();
    lazySet2.clear();
    bars.clear();
    rep(i, 0, FN) bars1d[i].clear();
    rep(i, 0, FN) rep(j, 0, FN) c2ds[i][j].clear();
    mapEdgeC1d.clear();
    rep(i, 0, FN) edges1d[i].clear();
    seudoEdges.clear();
    colorPairs.clear();

    appoint(yRotate(N - 2, N), "R'", "R2", "R");
    appoint(yRotate(0, 2), "L", "L2", "L'");
    appoint(zRotate(N - 2, N), "U", "U2", "U'");
    appoint(zRotate(0, 2), "D'", "D2", "D");
    appoint(xRotate(N - 2, N), "F", "F2", "F'");
    appoint(xRotate(0, 2), "B'", "B2", "B");


    appoint(zRotate(0, N), "y", "y2", "y'");
    appoint(yRotate(0, N), "x'", "x2", "x");
    appoint(xRotate(0, N), "z", "z2", "z'");


    if (N >= 6) {
        appoint(zRotate(N - 3, N), "u", "u2", "u'");
        appoint(zRotate(0, 3), "d'", "d2", "d");
        appoint(yRotate(N - 3, N), "r'", "r2", "r");
        appoint(yRotate(0, 3), "l", "l2", "l'");
        appoint(xRotate(N - 3, N), "f", "f2", "f'");
        appoint(xRotate(0, 3), "b'", "b2", "b");
    }

    if (N >= 8) {
        appoint(zRotate(N - 4, N), "3u", "3u2", "3u'");
        appoint(zRotate(0, 4), "3d'", "3d2", "3d");
        appoint(yRotate(N - 4, N), "3r'", "3r2", "3r");
        appoint(yRotate(0, 4), "3l", "3l2", "3l'");
        appoint(xRotate(N - 4, N), "3f", "3f2", "3f'");
        appoint(xRotate(0, 4), "3b'", "3b2", "3b");
    }


    for (auto const &item: perms) {
        permsP.emplace_back(item.second);
        permsS.emplace_back(item.first);
    }


    laterTurnSet = vector<string>{"R", "r", "l", "L", "U", "F", "D", "B", "x"};
    if (N >= 8) {
        laterTurnSet.emplace_back("3r");
        laterTurnSet.emplace_back("3l");
    }
    int tmpN = laterTurnSet.size();
    rep(i, 0, tmpN) {
        laterTurnSet.emplace_back(laterTurnSet[i] + "\'");
        laterTurnSet.emplace_back(laterTurnSet[i] + "2");
    }

    perms["lazyA"] = permFromStr(lazyA);
    perms["lazyA'"] = permFromStr(lazyAPrime);

    perms["lazyB"] = permFromStr(lazyB);
    perms["lazyB'"] = permFromStr(lazyBPrime);

    perms["lazyC"] = perms["lazyC'"] = permFromStr(lazyC);

    perms["lazyD"] = permFromStr(lazyD);
    perms["lazyD'"] = permFromStr(lazyDPrime);


    lazySet1 = alg{"U", "U2", "U'", "lazyA", "lazyA'", "lazyB", "lazyB'"};
    lazySet2 = alg{"U", "U2", "U'", "lazyC", "lazyC'", "lazyD", "lazyD'"};
    if (N == 6 || N == 8) {


        parityA = N == 6 ? alg{"r", "U2", "x", "r", "U2", "r", "U2",
                                   "x'", "l'", "U2", "l", "U2", "r'", "U2", "r", "U2", "r'", "U2", "r'"} :
                      alg{"3r", "U2", "x", "3r", "U2", "3r", "U2",
                          "x'", "3l'", "U2", "3l", "U2", "3r'", "U2", "3r", "U2", "3r'", "U2", "3r'"};
        perms["parityA"] = perms["parityA'"] = permFromStr(parityA);

        parityB = N == 6 ? alg{"r2", "R2", "U2", "r2", "R2", "u2", "r2", "R2", "u2", "U2"} :
                      alg{"3r2", "R2", "U2", "3r2", "R2", "3u2", "3r2", "R2", "3u2", "U2"};
        perms["parityB"] = perms["parityB'"] = permFromStr(parityB);
        lazySet1.emplace_back("parityA");
        lazySet1.emplace_back("parityA'");
        lazySet2.emplace_back("parityB");
        lazySet2.emplace_back("parityB'");

    }


    perm flip = composite(composite(composite(composite(composite(composite(perms["R"], perms["U"]),
                                                                  perms["R\'"]), perms["F"]), perms["R\'"]),
                                    perms["F\'"]), perms["R"]);
    perms["flip"] = flip;

    perm insertA = composite(composite(perms["R\'"], perms["U"]), perms["R"]);
    appoint(insertA, "insertA", "insertA2", "insertA\'");
    perm insertB = composite(composite(perms["R"], perms["U"]), perms["R\'"]);
    appoint(insertB, "insertB", "insertB2", "insertB\'");

    edgeSet = vector<string>{"y", "U", "u", "d", "D", "insertA", "insertB"};
    if (N >= 8) {
        edgeSet.emplace_back("3u");
        edgeSet.emplace_back("3d");
    }
    tmpN = edgeSet.size();
    rep(i, 0, tmpN) {
        edgeSet.emplace_back(edgeSet[i] + "\'");
        edgeSet.emplace_back(edgeSet[i] + "2");
    }
    edgeSet.emplace_back("z2");
    edgeSet.emplace_back("flip");

    edgeSet2 = vector<string>{"y", "U", "u", "d", "D"};
    if (N >= 8) {
        edgeSet2.emplace_back("3u");
        edgeSet2.emplace_back("3d");
    }
    tmpN = edgeSet2.size();
    rep(i, 0, tmpN) {
        edgeSet2.emplace_back(edgeSet2[i] + "\'");
        edgeSet2.emplace_back(edgeSet2[i] + "2");
    }
    edgeSet2.emplace_back("flip");

    simpleSet = vector<string>{"R", "U", "F", "L", "D", "B"};
    tmpN = simpleSet.size();
    rep(i, 0, tmpN) {
        simpleSet.emplace_back(simpleSet[i] + "\'");
        simpleSet.emplace_back(simpleSet[i] + "2");
    }

    pureU.emplace_back("u");
    pureU.emplace_back("d");
    pureU.emplace_back("y");
    if (N >= 8) {
        pureU.emplace_back("3u");
        pureU.emplace_back("3d");
    }
    tmpN = pureU.size();
    rep(i, 0, tmpN) {
        pureU.emplace_back(pureU[i] + "\'");
        pureU.emplace_back(pureU[i] + "2");
    }
    pureU.emplace_back("z2");

    int sd = 1;
    render(0, 1, sd, N - sd, sd, N - sd, 'b');
    render(N - 1, N, sd, N - sd, sd, N - sd, 'g');
    render(sd, N - sd, 0, 1, sd, N - sd, 'o');
    render(sd, N - sd, N - 1, N, sd, N - sd, 'r');
    render(sd, N - sd, sd, N - sd, 0, 1, 'y');
    render(sd, N - sd, sd, N - sd, N - 1, N, 'w');


    int st[3], ed[3];
    rep_(st[0], 0, N) rep_(ed[0], 1, N + 1)
            rep_(st[1], 0, N) rep_(ed[1], 1, N + 1)
                    rep_(st[2], 0, N) rep_(ed[2], 1,
                                           N + 1)
                            rep(i, 0,
                                3) rep(
                                        j, 0, 3) rep(k, 0, 3)
                                        if (st[i] + 1 == ed[i] && (st[i] == 0 || st[i] == N - 1) &&
                                            st[j] == 2 && ed[j] == N - 2
                                            && st[k] + 1 == ed[k] && st[k] >= 2 && st[k] < N - 2) {

                                            bar tmp;
                                            rep(ii, 0, 3) {
                                                tmp.st[ii] = st[ii];
                                                tmp.ed[ii] = ed[ii];
                                            }
                                            bars.emplace_back(tmp);
                                            bars1d[abs(2 * st[k] - N + 1)].emplace_back(tmp);
                                        }
//    rep(i, 0, N) printf("%d\n", bars1d[i].size());

    rep(i, 0, N) rep(j, 0, N) rep(k, 0, N)
                if (surface(cood(i, j, k))) {
                    auto c2d = cood_2D(cood(i, j, k));
                    c2ds[c2d.first][c2d.second].emplace_back(cood(i, j, k));
                }
//    rep(i, 0, N) {
//        rep(j, 0, N)
//            printf("%d ", c2ds[i][j].size());
//        printf("\n");
//    }
    int c[3];
    rep_(c[0], 0, N) rep_(c[1], 0, N) rep_(c[2], 0, N) rep(i, 0, 3) rep(j, 0, 3) rep(k, 0, 3)
                            if (
                                    (c[i] == 1 || c[i] == N - 2) && (c[j] == 1 || c[j] == N - 2) && c[k] >= 1 &&
                                    c[k] <= N - 2) {
                                int shortAxis1[3]{0}, shortAxis2[3]{0}, longAxis[3]{0}, ans[3];
                                if (c[i] == 1) shortAxis1[i] -= 1; else shortAxis1[i] += 1;
                                if (c[j] == 1) shortAxis2[j] -= 1; else shortAxis2[j] += 1;
                                longAxis[k] = 1;
                                cross(shortAxis1, shortAxis2, ans);
                                if (ans[k] != 1) continue;

                                cood a, b;
                                a = b = cood(c[0], c[1], c[2]);
                                a[i] = a[i] + shortAxis1[i];
                                b[j] = b[j] + shortAxis2[j];

                                if (c[k] == 1 || c[k] == N - 2) {
                                    if (c[k] == N - 2) seudoEdges.emplace_back(mp(a, b));
                                    else seudoEdges.emplace_back(mp(b, a));
                                    continue;
                                }
                                edges1d[c[k]].emplace_back(mp(a, b));
                                mapEdgeC1d[mp(a, b)] = c[k];
                                edges1d[N - 1 - c[k]].emplace_back(mp(b, a));
                                mapEdgeC1d[mp(b, a)] = N - 1 - c[k];
                            }

//    rep(i, 0, N) cout << edges1d[i].size() << endl;

    rep(i, 0, 6) rep(j, 0, 6) if (i + j != 5 && i != j) colorPairs.emplace_back(mp(colors[i], colors[j]));

}


state apply(state s, perm p) {
    state ret = s;
    for (const auto &item: p.p) {
//        printf("-");
        ret.a[item.first.x][item.first.y][item.first.z] =
                s.a[item.second.x][item.second.y][item.second.z];
    }
//    printf("\n");
    return ret;
}

state apply(state s, alg al) {
    for (auto const &a: al) {
        s = apply(s, perms[a]);
    }
    return s;
}

//auto fstate = ofstream("state.txt");

//void cout_(char ch) {
//    if (ch == 'o' || ch == 'r' || ch == 'g' || ch == 'b' || ch == 'y' || ch == 'w') {
//        fstate << ch << " ";
//    }
//}
//
//void show(state State) {
//    rep(i, 1, N - 1) {
//        rep(j, 0, N) fstate << " ";
//        rep(j, 0, N) cout_( State.a[i][j][N - 1]);
//        fstate << endl;
//    }
//    rep(i, 1, N - 1) {
//        rep(j, 0, N) cout_(State.a[j][0][N - 1 - i]);
//        rep(j, 0, N) cout_(State.a[N - 1][j][N - 1 - i]);
//        rep(j, 0, N) cout_(State.a[N - 1 - j][N - 1][N - 1 - i]);
//        rep(j, 0, N) cout_(State.a[0][N - 1 - j][N - 1 - i]);
//        fstate << endl;
//    }
//    for (int i = N - 2; i >= 1; i--) {
//        rep(j, 0, N) fstate << " ";
//        rep(j, 0, N) cout_(State.a[i][j][0]);
//        fstate << endl;
//    }
//}


alg bd_bfs(state State1, state State2, vector<string> turns, int kTh) {
    if (State1 == State2) return vector<string>{};
    queue<state> q1, q2;
    map<state, alg> record1, record2;
    q1.push(State1);
    q2.push(State2);
    record1[State1] = vector<string>{};
    record2[State2] = vector<string>{};

    for (int k = 1; k <= kTh; k++) {
//        cout << k << endl;
//        cout << q1.size() << " " << q2.size() << endl;
        while (!q1.empty()) {
            auto head = q1.front();
            if (record1[head].size() == k) break;
            for (auto const &turn: turns) {
                auto new1 = apply(head, perms[turn]);
                if (record1.count(new1) == 0) {
                    record1[new1] = record1[head];
                    record1[new1].emplace_back(turn);
                    q1.push(new1);
                }
            }
            q1.pop();
        }
        state best_new2;
        int best = 999;
        while (!q2.empty()) {
            auto head = q2.front();
            if (record2[head].size() == k) break;
            for (auto const &turn: turns) {
                auto new2 = apply(head, perms[turn]);
                if (record2.count(new2) == 0) {
                    record2[new2] = record2[head];
                    record2[new2].emplace_back(turn);
                    q2.push(new2);
                    if (record1.count(new2) != 0 && record1[new2].size() < best) {
                        best = record1[new2].size();
                        best_new2 = new2;
                    }
                }
            }
            q2.pop();
        }
        if (best != 999) {
            auto ans = record1[best_new2], inv = inverse(record2[best_new2]);
            ans.insert(ans.end(), inv.begin(), inv.end());
            return ans;
        }
    }
    return vector<string>{"too long"};
}


void barWithTh(state oState, int stage, bar Bar, vector<bar> barScheme, int color, int &shortestLen, alg &shortestAlg,
               int &currentColor, int kTh, bool preserve) {
    alg subAlg;

    state State = oState;
    bool viable = true;
    for (;;) {
        state State1, State2;
        rep(s, 0, stage)rep(i, barScheme[s].st[0], barScheme[s].ed[0])rep(j, barScheme[s].st[1],
                                                                          barScheme[s].ed[1])rep(k,
                                                                                                 barScheme[s].st[2],
                                                                                                 barScheme[s].ed[2]) {
                        State2.a[i][j][k] = State1.a[i][j][k] = oState.a[i][j][k];
                    }
        vector<cood> empties;
        rep(i, Bar.st[0], Bar.ed[0])
            rep(j, Bar.st[1], Bar.ed[1])
                rep(k, Bar.st[2], Bar.ed[2]) {
                    if (State.a[i][j][k] != colors[color]) {
                        empties.emplace_back(cood(i, j, k));
                    } else if (preserve) {
                        State1.a[i][j][k] = colors[color];
                        State2.a[i][j][k] = colors[color];
                    }
                }
        if (empties.empty()) break;

        sort(empties.begin(), empties.end());

        auto c = empties[0];

        if (!preserve) {
            rep(i, Bar.st[0], Bar.ed[0])rep(j, Bar.st[1], Bar.ed[1])rep(k, Bar.st[2], Bar.ed[2]) {
                        if (State.a[i][j][k] == colors[color] && cood(i, j, k).dis() <= c.dis()) {
                            State1.a[i][j][k] = colors[color];
                            State2.a[i][j][k] = colors[color];
                        }
                    }
        }

        State1.a[c[0]][c[1]][c[2]] = colors[color];
        auto c2d = cood_2D(c);

        if (stage >= N - 4 && stage < 2 * N - 8 && N % 2 == 1 && c2d.first == N / 2 && c2d.second == N / 2) {
            viable = false;
            break;
        }

        if (stage >= 3 * N - 12 && N % 2 == 1 && c2d.first == N / 2 && c2d.second == N / 2) {
            viable = false;
            break;
        }


        for (auto const &c2: c2ds[c2d.first][c2d.second]) {
            bool flag = true;
            rep(i, 0, stage) if (barScheme[i].in(c2)) {
                    flag = false;
                    break;
                }
            if (!flag || Bar.in(c2)) {
                continue;
            }
            if (State.a[c2[0]][c2[1]][c2[2]] == colors[color]) {
                State2.a[c2[0]][c2[1]][c2[2]] = colors[color];
                break;
            }
        }

        vector<string> turnSet = stage < 2 * N - 8 ? permsS : laterTurnSet;

        auto al = bd_bfs(State2, State1, turnSet, kTh);
        if (al[0] == "too long") {
            viable = false;
            break;
        }
        subAlg.insert(subAlg.end(), al.begin(), al.end());

        if (subAlg.size() > shortestLen) {
            viable = false;
            break;
        }

        State = apply(State, al);

    }

    if (!viable) return;

    state State1, State2;
    rep(s, 0, stage)rep(i, barScheme[s].st[0], barScheme[s].ed[0])rep(j, barScheme[s].st[1],
                                                                      barScheme[s].ed[1])rep(k,
                                                                                             barScheme[s].st[2],
                                                                                             barScheme[s].ed[2]) {
                    State2.a[i][j][k] = State1.a[i][j][k] = oState.a[i][j][k];
                }
    rep(i, barScheme[stage].st[0], barScheme[stage].ed[0])rep(j, barScheme[stage].st[1],
                                                              barScheme[stage].ed[1])rep(k,
                                                                                         barScheme[stage].st[2],
                                                                                         barScheme[stage].ed[2]) {
                State1.a[i][j][k] = colors[color];
            }
    rep(i, Bar.st[0], Bar.ed[0])rep(j, Bar.st[1], Bar.ed[1])rep(k, Bar.st[2], Bar.ed[2]) {
                State2.a[i][j][k] = colors[color];
            }
    vector<string> turnSet = stage < 2 * N - 8 ? permsS : laterTurnSet;

    auto al = bd_bfs(State2, State1, turnSet, kTh);
    if (al.size() > 0 && al[0] == "too long") {
        return;
    }
    subAlg.insert(subAlg.end(), al.begin(), al.end());
//                print(subAlg);
//                cnt++;

    if (subAlg.size() < shortestLen) {
        shortestLen = subAlg.size();
        shortestAlg = subAlg;
        currentColor = color;
    }
}



state solveCenter(state State) {


    map<char, int> colorMap;
    rep(i, 0, 6) colorMap[colors[i]] = i;

    map<char, cood> mapColorCood;
    map<cood, char> mapCoodColor;
    map<pair<pair<char, char>, char>, char> nextColor;

    mapColorCood['w'] = cood(0, 0, 1);
    mapColorCood['y'] = cood(0, 0, -1);
    mapColorCood['r'] = cood(0, 1, 0);
    mapColorCood['o'] = cood(0, -1, 0);
    mapColorCood['g'] = cood(1, 0, 0);
    mapColorCood['b'] = cood(-1, 0, 0);

    for (auto const &c: colors)
        mapCoodColor[mapColorCood[c]] = c;

    for (auto const &c1: colors)
        for (auto const &c2: colors)
            for (auto const &c3: colors) {
                int v1[3], v2[3];
                rep(i, 0, 3) {
                    v1[i] = mapColorCood[c3][i] - mapColorCood[c2][i];
                    v2[i] = mapColorCood[c2][i] - mapColorCood[c1][i];
                }
                int v[3] = {v1[1] * v2[2] - v1[2] * v2[1], v1[2] * v2[0] - v1[0] * v2[2],
                            v1[0] * v2[1] - v1[1] * v2[0]};
                rep(i, 0, 3) v[i] /= 2;
                nextColor[mp(mp(c1, c2), c3)] = mapCoodColor[cood(v[0], v[1], v[2])];
            }


//            cout << nextColor[mp(mp('g', 'w'), 'b')] << " for check\n";
    state oState = State;
    vector<bar> barScheme;
    for (auto const &Bar: bars) {
        if (Bar.st[1] == 0 && Bar.ed[1] == 1 && Bar.st[2] + 1 == Bar.ed[2]) barScheme.emplace_back(Bar);
    }
    sort(barScheme.begin(), barScheme.end());

    for (auto const &Bar: bars) {
        if (Bar.st[1] == N - 1 && Bar.ed[1] == N && Bar.st[2] + 1 == Bar.ed[2]) barScheme.emplace_back(Bar);
    }
    sort(barScheme.begin() + N - 4, barScheme.end());

    for (auto const &Bar: bars) {
        if (Bar.st[2] == 0 && Bar.ed[2] == 1 && Bar.st[1] + 1 == Bar.ed[1]) barScheme.emplace_back(Bar);
    }
    sort(barScheme.begin() + 2 * N - 8, barScheme.end());

    for (auto const &Bar: bars) {
        if (Bar.st[0] == 0 && Bar.ed[0] == 1 && Bar.st[1] + 1 == Bar.ed[1]) barScheme.emplace_back(Bar);
    }
    sort(barScheme.begin() + 3 * N - 12, barScheme.end());


    for (auto const &Bar: bars) {
        if (Bar.st[0] == N - 1 && Bar.ed[0] == N && Bar.st[1] + 1 == Bar.ed[1] && Bar.st[1] <= (N - 1) / 2)
            barScheme.emplace_back(Bar);
    }
    sort(barScheme.begin() + 4 * N - 16, barScheme.end());

    for (auto const &Bar: bars) {
        if (Bar.st[0] == N - 1 && Bar.ed[0] == N && Bar.st[1] + 1 == Bar.ed[1] && Bar.st[1] > (N - 1) / 2)
            barScheme.emplace_back(Bar);
    }
    sort(barScheme.begin() + 4 * N - 16 + (N - 3) / 2, barScheme.end());


    int currentColor;
    auto tmp = barScheme[2].get1d();
    int firstColor;
    rep(stage, 0, barScheme.size()) {
        int bars1d_current = barScheme[stage].get1d();

        vector<int> viableColor;

        if (stage == 0) {
            rep(i, 0, 6) viableColor.emplace_back(i);
        } else if (stage == N - 4) {
            firstColor = currentColor;
            f << endl;
            outText("\n");
            cout << endl;
            viableColor.emplace_back(5 - currentColor);
        } else if (stage == 2 * N - 8) {
            f << endl;
            outText("\n");
            cout << endl;
            rep(i, 0, 6) if (i != currentColor && 5 - i != currentColor) viableColor.emplace_back(i);
        } else if (stage == 3 * N - 12) {
            f << endl;
            outText("\n");
            cout << endl;
            viableColor.emplace_back(
                    colorMap[nextColor[mp(mp(colors[firstColor], colors[currentColor]), colors[5 - firstColor])]]);
        } else if (stage == 4 * N - 16) {
//            cout << clock() - start << endl;
            f << endl;
            outText("\n");
            cout << endl;
            viableColor.emplace_back(5 - currentColor);
        } else {
            viableColor.emplace_back(currentColor);
        }

        int shortestLen = 999;
        alg shortestAlg;


        for (int kTh = 1;; kTh++) {
            cout << "k: " << kTh << endl;
            rep(pre, 0, 2) {
                for (auto const &Bar: bars1d[bars1d_current]) {

                    bool flag = true;
                    rep(i, 0, stage) if (Bar.intersect(barScheme[i])) {
                            flag = false;
                            break;
                        }
                    if (!flag) continue;
                    for (auto const &color: viableColor) {
                        barWithTh(oState, stage, Bar, barScheme, color, shortestLen, shortestAlg, currentColor, kTh,
                                  1 - pre);
                        if (kTh >= 3 && shortestLen != 999) break;
                    }
                    if (kTh >= 3 && shortestLen != 999) break;
                }
                if (shortestLen != 999) break;
            }
            if (shortestLen != 999) break;
        }



//        rep(i, 0, parSolvers.size()) parSolvers[i].join();

//        cout << "##################################";
        print(shortestAlg, f);

//        cout << "its inverse: " << endl;
//        print(inverse(shortestAlg));
        oState = apply(oState, shortestAlg);

    }
    return oState;

}

void edgeWithTh(pair<char, char> colorPair, state oState, int kTh, int &shortestLen, alg &shortestAlg,
                pair<char, char> &chosenPair, map<pair<char, char>, bool> &paired) {
    state State1, State2;
    rep(z, 2, N - 2) {
        State1.a[N - 1][1][z] = colorPair.first;
        State1.a[N - 2][0][z] = colorPair.second;
    }
    rep(i, 0, N) for (auto const &Edge: edges1d[i]) {
            if (oState[Edge.first] == colorPair.first && oState[Edge.second] == colorPair.second) {
                State2[Edge.first] = colorPair.first;
                State2[Edge.second] = colorPair.second;
            }
        }
//
//    show(State2);
//    show(State1);
    auto al = bd_bfs(State2, State1, edgeSet, kTh);
    if (al.size() > 0 && al[0] == "too long") {
//        cout << "too long" << endl;
        return;
    }
//    print_(al);


    oState = apply(oState, al);
    vector<alg> algs;
    algs.emplace_back(vector<string>{"L'", "U", "L"});
    algs.emplace_back(vector<string>{"L'", "U'", "L"});
    algs.emplace_back(vector<string>{"L'", "U2", "L"});
    algs.emplace_back(vector<string>{"U'", "L'", "U", "L"});

    algs.emplace_back(vector<string>{"z2", "R", "U", "R'", "y"});
    algs.emplace_back(vector<string>{"z2", "R", "U2", "R'", "y"});
    algs.emplace_back(vector<string>{"z2", "R", "U'", "R'", "y"});
    algs.emplace_back(vector<string>{"z2", "U", "R", "U'", "R'", "y"});


    for (auto const &Alg: algs) {
        auto cState = apply(oState, Alg);
        bool flag = !paired.count(mp(cState.a[N - 1][1][2], cState.a[N - 2][0][2]));

        if (flag) {
            al.insert(al.end(), Alg.begin(), Alg.end());
            break;
        }

    }


    if (al.size() < shortestLen) {
        shortestAlg = al;
        shortestLen = al.size();
        chosenPair = colorPair;
    }

}


state solveEdge(state State) {

    map<pair<char, char>, bool> paired;
    pair<char, char> chosenPair;
//    ofstream f2("alg.txt", ios_base::app);
    stringstream &f2 = f;
    f2 << endl;
    outText("\n");
    auto oState = State;
    rep(k, 0, 8) {
        int shortestLen = 999;
        alg shortestAlg;
        for (int kTh = 1;; kTh++) {
            cout << "k: " << kTh << endl;
            for (auto const &colorPair: colorPairs) {
                if (paired.count(colorPair)) continue;

                edgeWithTh(colorPair, oState, kTh, shortestLen, shortestAlg, chosenPair, paired);
                if (shortestLen != 999) break;

            }
            if (shortestLen != 999) break;
        }

        print_(shortestAlg);
        paired[chosenPair] = true;
        paired[mp(chosenPair.second, chosenPair.first)] = true;

        print(shortestAlg, f2);
        oState = apply(oState, shortestAlg);


    }

    state State1, State2;
    rep(i, 0, N) rep(j, 0, N) rep(k, 0, N)
                if (center(cood(i, j, k))) {
                    State2.a[i][j][k] = oState.a[i][j][k];
                    State1.a[i][j][k] = State.a[i][j][k];
                }
    auto al = bd_bfs(State2, State1, pureU, 3);
    oState = apply(oState, al);
    print(al, f2);

    rep(k, 0, 3) {
        int shortestLen = 999;
        alg shortestAlg;
        for (int kTh = 1;; kTh++) {
            for (auto const &colorPair: colorPairs) {
                if (paired[colorPair]) continue;
                State1 = State2 = state();
                rep(i, 0, N) rep(j, 0, N) rep(k, 2, N - 2)if (center(cood(i, j, k))) {
                                State2.a[i][j][k] = oState.a[i][j][k];
                                State1.a[i][j][k] = oState.a[i][j][k];
                            }
                rep(z, 2, N - 2) {
                    if (k >= 1) {
                        State1.a[0][1][z] = oState.a[0][1][z];
                        State1.a[1][0][z] = oState.a[1][0][z];
                        State2.a[0][1][z] = oState.a[0][1][z];
                        State2.a[1][0][z] = oState.a[1][0][z];
                    }
                    if (k == 2) {
                        State1.a[0][N - 2][z] = oState.a[0][N - 2][z];
                        State1.a[1][N - 1][z] = oState.a[1][N - 1][z];
                        State2.a[0][N - 2][z] = oState.a[0][N - 2][z];
                        State2.a[1][N - 1][z] = oState.a[1][N - 1][z];
                    }

                    State1.a[N - 1][1][z] = colorPair.first;
                    State1.a[N - 2][0][z] = colorPair.second;
                }
                rep(i, 0, N) for (auto const &Edge: edges1d[i]) {
                        if (oState[Edge.first] == colorPair.first && oState[Edge.second] == colorPair.second) {
                            State2[Edge.first] = colorPair.first;
                            State2[Edge.second] = colorPair.second;
                        }
                    }

                auto al = bd_bfs(State2, State1, edgeSet2, kTh);
                if (al.size() > 0 && al[0] == "too long") continue;
                if (al.size() < shortestLen) {
                    shortestAlg = al;
                    shortestLen = al.size();
                    chosenPair = colorPair;
                }
            }
            if (shortestLen != 999) break;


        }

        if (k != 2) shortestAlg.emplace_back("y");
        print(shortestAlg, f2);

        paired[chosenPair] = true;
        paired[mp(chosenPair.second, chosenPair.first)] = true;

        oState = apply(oState, shortestAlg);
    }

    if (N == 9 && oState.a[N - 1][N - 2][3] != oState.a[N - 1][N - 2][4]) {
        alg al = vector<string>{"z'", "3r", "U2", "x", "3r", "U2", "3r", "U2",
                                "x'", "3l'", "U2", "3l", "U2", "3r'", "U2", "3r", "U2", "3r'", "U2", "3r'", "z"};
        oState = apply(oState, al);
        print(al, f2);
    }

    if (oState.a[N - 1][N - 2][3] != oState.a[N - 1][N - 2][2]) {
        alg al = vector<string>{"z'", "r", "U2", "x", "r", "U2", "r", "U2",
                                "x'", "l'", "U2", "l", "U2", "r'", "U2", "r", "U2", "r'", "U2", "r'", "z"};
        oState = apply(oState, al);
        print(al, f2);
    }
//    f2.close();
    return oState;

}

void work(char c1, char c2, state &State2, state &State) {
    rep(i, 0, N) for (auto const &Edge: edges1d[i]) {
            if (State[Edge.first] == c1 && State[Edge.second] == c2) {
                State2[Edge.first] = State[Edge.first];
                State2[Edge.second] = State[Edge.second];
            }
        }
    for (auto const &Edge: seudoEdges) {
        if (State[Edge.first] == c1 && State[Edge.second] == c2) {
            State2[Edge.first] = State[Edge.first];
            State2[Edge.second] = State[Edge.second];
        }
    }
}

void solveThree(state State) {
//    ofstream f3("alg.txt", ios_base::app);
    stringstream &f3 = f;
    state State1, State2;
    auto btm = State.a[2][2][0];
    rep(i, 1, N - 1) rep (j, 1, N - 1) if (!center(cood(i, j, 0)) && ((j >= 2 && j < N - 2) || (i >= 2 && i < N - 2))) {
                State1.a[i][j][0] = State.a[2][2][0];
            }
    rep(i, 0, N) rep (j, 0, N) if (surface(cood(i, j, 1)) && ((j >= 2 && j < N - 2) || (i >= 2 && i < N - 2))) {
                State1.a[i][j][1] = State.a[i][j][2];
            }
    rep(i, 0, N) for (auto const &Edge: edges1d[i]) {
            if (State[Edge.first] == btm) {
                State2[Edge.first] = State[Edge.first];
                State2[Edge.second] = State[Edge.second];
            }
        }
//    show(State2);
//    show(State1);
    auto al = bd_bfs(State2, State1, simpleSet, 999);
    f3 << endl;
    outText("\n");

    print(al, f3);
    State = apply(State, al);

    // just lazy...
    char c1, c2;

    State2 = State1;
    c1 = State.a[2][0][2];
    c2 = State.a[0][2][2];
    rep(z, 1, N - 2) {
        State1.a[1][0][z] = c1;
        State1.a[0][1][z] = c2;
    }
    work(c1, c2, State2, State);
//    show(State2);
//    show(State1);
    al = bd_bfs(State2, State1, simpleSet, 999);
    print(al, f3);
    State = apply(State, al);

    State2 = State1;
    c1 = c2;
    c2 = State.a[2][N - 1][2];
    rep(z, 1, N - 2) {
        State1.a[0][N - 2][z] = c1;
        State1.a[1][N - 1][z] = c2;
    }
    work(c1, c2, State2, State);
//    show(State2);
//    show(State1);


    al = bd_bfs(State2, State1, simpleSet, 999);
    print(al, f3);
    State = apply(State, al);

    State2 = State1;
    c1 = c2;
    c2 = State.a[N - 1][2][2];
    rep(z, 1, N - 2) {
        State1.a[N - 2][N - 1][z] = c1;
        State1.a[N - 1][N - 2][z] = c2;
    }
    work(c1, c2, State2, State);
//    show(State2);
//    show(State1);

    al = bd_bfs(State2, State1, simpleSet, 999);


    print(al, f3);
    State = apply(State, al);

    State2 = State1;
    c1 = c2;
    c2 = State.a[2][0][2];
    rep(z, 1, N - 2) {
        State1.a[N - 1][1][z] = c1;
        State1.a[N - 2][0][z] = c2;
    }

    work(c1, c2, State2, State);
//    show(State2);
//    show(State1);


    al = bd_bfs(State2, State1, simpleSet, 999);
    print(al, f3);
    State = apply(State, al);

    State2 = State1;
    char topColor = State.a[2][2][N - 1];
    rep(i, 1, N - 1) rep(j, 1, N - 1) State1.a[i][j][N - 1] = topColor;
    rep(i, 0, N) rep(j, 0, N) rep(k, 0, N) if (State.a[i][j][k] == topColor) {
        State2.a[i][j][k] = topColor;
    }

    al = bd_bfs(State2, State1, lazySet1, 999);
    print(al, f3);
    State = apply(State, al);

    State2 = State1;
    rep(i, 0, N) rep(j, 0, N) State2.a[i][j][N - 2] = State.a[i][j][N - 2];
    rep(i, 0, N) rep(j, 0, N) State1.a[i][j][N - 2] = State1.a[i][j][1];
//    show(State2);
//    show(State1);
    al = bd_bfs(State2, State1, lazySet2, 999);
    print(al, f3);

}




void scramble(int M_) {


    M = M_;
    auto al = randomAlg();
    stringstream f("");
    print(al, f);
    cout << f.str() << endl;
    string cmd = "document.getElementById(\"2333\").value = `" + f.str() + "`";
//    cmd = "alert(`" + f.str() + "`)";
    cout << cmd << endl;
    emscripten_run_script(cmd.c_str());



}


void solve(int M_, string scr) {

    M = M_;
//    srand(1234);
    outText("\n");
    outText("\n");

    init();


//    auto a = readAlg("alg.txt");
//    ofstream ff("trans.txt");
//    print(a, ff);
//    return 0;


//    int DEBUG = 0;
//
//    if (DEBUG) {
//        ifstream f("alg.txt");

    stringstream f(scr);
    string s;
    while (f >> s) {
        State = apply(State, perms[s]);
    }
//    f.close();
//    show(State);



//    } else {


//
//    f = stringstream(""); //ofstream("alg.txt");
//
////    start = clock();
//    auto al = randomAlg();
//
//    print(al, f);
//    f << endl;
//    f << "R R R R R R R R R R R R" << endl;
//    State = apply(State, al);



//    if (argc == 2) {
//
//        auto ff = ifstream(string(argv[1]));
//
//        int n = N - 2;
//        map<char, int> colorMap;
//        rep(i, 0, 6) colorMap[colors[i]] = i;
//
//
//
////    void show(state State) {
////        rep(i, 1, N - 1) {
////            rep(j, 0, N) cout << " ";
////            rep(j, 0, N) cout_( State.a[i][j][N - 1]);
////            cout << endl;
////        }
////        rep(i, 1, N - 1) {
////            rep(j, 0, N) cout_(State.a[j][0][N - 1 - i]);
////            rep(j, 0, N) cout_(State.a[N - 1][j][N - 1 - i]);
////            rep(j, 0, N) cout_(State.a[N - 1 - j][N - 1][N - 1 - i]);
////            rep(j, 0, N) cout_(State.a[0][N - 1 - j][N - 1 - i]);
////            cout << endl;
////        }
////        for (int i = N - 2; i >= 1; i--) {
////            rep(j, 0, N) cout << " ";
////            rep(j, 0, N) cout_(State.a[i][j][0]);
////            cout << endl;
////        }
////    }
//
//        for (int i = 0; i < n; i++) {
//            for (int j = 0; j < n; j++) {
//                char ch;
//                ff >> ch;
//
//                while (ch == ' ' || ch == '\n') ff >> ch;
//                int c = (colorMap[ch]);
//                int x = 1 + i;
//                int y = 1 + j;
//                int z = n + 1;
//                State.a[x][y][z] = ch;
//            }
//
//        }
//
//        for (int i = 0; i < n; i++) {
//            for (int j = 0; j < n; j++) {
//                char ch;
//                ff >> ch;
//
//                while (ch == ' ' || ch == '\n') ff >> ch;
//                int c = (colorMap[ch]);
//                int x = 1 + j;
//                int y = 0;
//                int z = n - i;
//                State.a[x][y][z] = ch;
//            }
//            for (int j = 0; j < n; j++) {
//                char ch;
//                ff >> ch;
//
//                while (ch == ' ' || ch == '\n') ff >> ch;
//                int c = (colorMap[ch]);
//                int x = n + 1;
//                int y = 1 + j;
//                int z = n - i;
//                State.a[x][y][z] = ch;
//            }
//            for (int j = 0; j < n; j++) {
//                char ch;
//                ff >> ch;
//
//                while (ch == ' ' || ch == '\n') ff >> ch;
//                int c = (colorMap[ch]);
//                int x = n - j;
//                int y = n + 1;
//                int z = n - i;
//                State.a[x][y][z] = ch;
//            }
//            for (int j = 0; j < n; j++) {
//                char ch;
//                ff >> ch;
//
//                while (ch == ' ' || ch == '\n') ff >> ch;
//                int c = (colorMap[ch]);
//                int x = 0;
//                int y = n - j;
//                int z = n - i;
//                State.a[x][y][z] = ch;
//            }
//        }
//
//        for (int i = 0; i < n; i++) {
//            for (int j = 0; j < n; j++) {
//                char ch;
//                ff >> ch;
//
//                while (ch == ' ' || ch == '\n') ff >> ch;
//                int c = (colorMap[ch]);
//                int x = n - i;
//                int y = 1 + j;
//                int z = 0;
//                State.a[x][y][z] = ch;
//            }
//
//        }
//    }
//    show(State);
    State = solveCenter(State);


//    cout << clock() - start << endl;
//    f.close();
//    }

//    f = stringstream("");
    State = solveEdge(State);


    solveThree(State);

//    return f.str();


//    return 0;
}

string reverseAlg(string s) {
    alg a = inverse(algFromStr(s));
    string out = "";
    for (string turn: a) {
        out = out + turn + " ";
    }
    return out;
}

EMSCRIPTEN_BINDINGS(module) {

  emscripten::function("solve", &solve);
  emscripten::function("scramble", &scramble);
  emscripten::function("reverse", &reverseAlg);


  // register bindings for std::vector<int> and std::map<int, std::string>.
//  register_vector<int>("vector<int>");
//  register_map<int, std::string>("map<int, string>");
}