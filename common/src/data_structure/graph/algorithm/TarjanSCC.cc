/**
 * @file TarjanSCC.cc
 * @brief Tarjan's Strongly Connected Components algorithm implementation
 * @details A single DFS pass assigns each node a unique index and computes
 *          a lowlink value (the smallest index reachable via the DFS subtree
 *          and at most one back edge). When a node's index equals its lowlink,
 *          it is the root of an SCC â€?all nodes on the stack above it form
 *          that SCC. Time complexity: O(V+E).
 */

#include <cppforge/data_structure/graph/algorithm/TarjanSCC.hpp>

#include <algorithm>
#include <stack>

namespace cppforge::data_structure::graph::algorithm
{
    namespace
    {
        /// @brief Recursive DFS for Tarjan's SCC algorithm
        /// @param index Discovery order index for each node
        /// @param lowlink Lowest index reachable from each node
        /// @param on_stack Whether a node is currently on the recursion stack
        /// @param st Stack of nodes in the current DFS path
        /// @param current_index Counter for assigning discovery order
        void tarjanDFS(const Graph& graph, int32_t u,
                       std::vector<int32_t>& index,
                       std::vector<int32_t>& lowlink,
                       std::vector<bool>& on_stack,
                       std::stack<int32_t>& st,
                       int32_t& current_index,
                       SCCResult& result)
        {
            const size_t u_idx = static_cast<size_t>(u);
            index[u_idx] = current_index;
            lowlink[u_idx] = current_index;
            ++current_index;
            st.push(u);
            on_stack[u_idx] = true;

            for (const auto& edge : graph.getAdjList(u))
            {
                const int32_t v = edge.to();
                const size_t v_idx = static_cast<size_t>(v);

                if (index[v_idx] == -1)
                {
                    // Tree edge: recurse, then update lowlink
                    tarjanDFS(graph, v, index, lowlink, on_stack, st,
                              current_index, result);
                    lowlink[u_idx] = std::min(lowlink[u_idx], lowlink[v_idx]);
                }
                else if (on_stack[v_idx])
                {
                    // Back/cross edge to a node on the stack
                    lowlink[u_idx] = std::min(lowlink[u_idx], index[v_idx]);
                }
            }

            // If u is the root of an SCC, pop all nodes in this SCC
            if (lowlink[u_idx] == index[u_idx])
            {
                std::vector<int32_t> component;
                while (true)
                {
                    const int32_t w = st.top();
                    st.pop();
                    on_stack[static_cast<size_t>(w)] = false;
                    component.push_back(w);
                    result.component_id[static_cast<size_t>(w)] =
                        static_cast<int32_t>(result.components.size());
                    if (w == u) break;
                }
                result.components.push_back(std::move(component));
            }
        }
    }

    SCCResult TarjanSCC::compute(const Graph& graph)
    {
        const int32_t n = graph.getNodeCount();
        SCCResult result;
        result.component_id.assign(static_cast<size_t>(n), -1);

        if (n == 0) return result;

        std::vector<int32_t> index(static_cast<size_t>(n), -1);
        std::vector<int32_t> lowlink(static_cast<size_t>(n), -1);
        std::vector<bool> on_stack(static_cast<size_t>(n), false);
        std::stack<int32_t> st;
        int32_t current_index = 0;

        // Start DFS from each unvisited node
        for (int32_t i = 0; i < n; ++i)
        {
            if (index[static_cast<size_t>(i)] == -1)
            {
                tarjanDFS(graph, i, index, lowlink, on_stack, st,
                          current_index, result);
            }
        }

        return result;
    }
}
