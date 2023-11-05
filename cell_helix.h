// 由内向外的螺旋数组，用于以指定坐标点为中心点，遍历周围坐标点的场景
// 当前是 7 * 7 的范围，如需更大范围可以修改 __helix_radius
// eg. 怪物掉落、传送目标点遍历寻找可用点

/*
42	43	44	45	46	47	48
41	20	21	22	23	24	25
40	19	6	7	8	9	26
39	18	5	0	1	10	27
38	17	4	3	2	11	28
37	16	15	14	13	12	29
36	35	34	33	32	31	30
 */

// 坐标点
struct cell {
    unsigned x = 0;
    unsigned y = 0;
};

static constexpr int __helix_radius = 3;
static constexpr int __helix_size = (__helix_radius * 2 + 1) * (__helix_radius * 2 + 1);
static cell __cell_helix[__helix_size];

static int _helix(int x, int y) {
    int t = std::max(std::abs(x), std::abs(y));
    int u = t + t;
    int v = u - 1;

    v = v * v + u;
    if( x == -t )
        v += u + t - y;
    else if( y == -t )
        v += 3 * u + x - t;
    else if( y == t )
        v += t - x;
    else
        v += y - t;

    return v - 1;
}

static bool _init_helix() {
    for(int y = -1 * __helix_radius; y <= __helix_radius; ++y) {
        for(int x = -1 * __helix_radius; x <= __helix_radius; ++x) {
            cell& cpt = __cell_helix[_helix(x, y)];
            cpt.x = x;
            cpt.y = y;
        }
    }

    return true;
}

static bool __call_init_helix = _init_helix();
