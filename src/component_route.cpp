//
// Created by Murph on 2020/9/18.
//

#include "weavess/component.h"

namespace weavess {
    void ComponentSearchRouteGreedy::RouteInner(unsigned int query, std::vector<Index::Neighbor> &pool,
                                                std::vector<unsigned int> &res) {
        const auto L = index->getParam().get<unsigned>("L_search");
        const auto K = index->getParam().get<unsigned>("K_search");

        std::vector<char> flags(index->getBaseLen());

        int k = 0;
        while (k < (int) L) {
            int nk = L;

            if (pool[k].flag) {
                pool[k].flag = false;
                unsigned n = pool[k].id;

                // 查找邻居的邻居
                for (unsigned m = 0; m < index->getFinalGraph()[n][0].size(); ++m) {
                    unsigned id = index->getFinalGraph()[n][0][m];

                    if (flags[id])continue;
                    flags[id] = 1;
                    float dist = index->getDist()->compare(index->getQueryData() + index->getQueryDim() * query,
                                                           index->getBaseData() + index->getBaseDim() * id,
                                                           (unsigned) index->getBaseDim());
                    index->addDistCount();

                    if (dist >= pool[L - 1].distance) continue;
                    Index::Neighbor nn(id, dist, true);
                    int r = Index::InsertIntoPool(pool.data(), L, nn);

                    //if(L+1 < retset.size()) ++L;
                    if (r < nk)nk = r;
                }
                //lock to here
            }
            if (nk <= k)k = nk;
            else ++k;
        }

        for (size_t i = 0; i < K; i++) {
            res[i] = pool[i].id;
        }
    }

    void ComponentSearchRouteGuide::RouteInner(unsigned int query, std::vector<Index::Neighbor> &pool,
                                               std::vector<unsigned int> &res) {
        const auto L = index->getParam().get<unsigned>("L_search");
        const auto K = index->getParam().get<unsigned>("K_search");

        boost::dynamic_bitset<> flags{index->getBaseLen(), 0};

        for(int i = 0; i < pool.size(); i ++) {
            flags[pool[i].id] = true;
        }

        int k = 0;
        while (k < (int)L) {
            int nk = L;

            if (pool[k].flag) {
                pool[k].flag = false;
                unsigned n = pool[k].id;

                Index::Tnode node = index->Tn[n];

                while(node.left) {
                    if((index->getBaseData() + n * index->getBaseDim())[node.div_dim] < (index->getQueryData() + query * index->getQueryDim())[node.div_dim]) {
                        node = *(node.left);
                    }else {
                        node = *(node.right);
                    }
                }

                for(int m = 0; m < node.val.size(); m ++) {
                    unsigned id = node.val[m];
                    if (flags[id]) continue;
                    flags[id] = true;
                    float dist = index->getDist()->compare(index->getQueryData() + query * index->getQueryDim(),
                                                           index->getBaseData() + id * index->getBaseDim(),
                                                           index->getBaseDim());
                    //dist_cout++;
                    if (dist >= pool[L - 1].distance) continue;
                    Index::Neighbor nn(id, dist, true);
                    int r = Index::InsertIntoPool(pool.data(), L, nn);

                    // if(L+1 < retset.size()) ++L;
                    if (r < nk) nk = r;
                }
            }
            if (nk <= k)
                k = nk;
            else
                ++k;
        }
        for (size_t i = 0; i < K; i++) {
            res[i] = pool[i].id;
        }
    }
}