#ifndef WGRAPH_H
#define WGRAPH_H

#include "../graph/graph.h"
#include "../graph/bfs.h"
#include "../graph/dfs.h"
#include "../graph/eulerian.h"
#include "wedge.h"
#include "ds.h"
#include "dary_heap.h"

#include <algorithm>
#include <unordered_map>
#include <map>
#include <set>


template <class Vertex>
class wgraph : public graph<Vertex> {
public:

	// default constructor
	wgraph() { }

	// return true and add edge if not already in graph
	bool addEdge(const Vertex &v, const Vertex &w, double c) {
		if (!graph<Vertex>::isVertex(v) || !graph<Vertex>::isVertex(w)) { return false; }
		if (graph<Vertex>::isEdge(v, w)) { return false; }

		graph<Vertex>::addEdge(v, w);
		_c[Edge<Vertex>(v, w)] = c;
		_c[Edge<Vertex>(w, v)] = c;
		return true;
	}

	// return true and add edge if not already in graph
	bool addEdge(const Edge<Vertex> &e, double c) {
		return addEdge(e.v, e.w, c);
	}

	// return true and add edge if not already in graph
	bool addEdge(const WEdge<Vertex> &e) {
		return addEdge(e.v, e.w, e.c);
	}

	// removes edge if in graph
	void removeEdge(const Vertex &v, const Vertex &w) {
		graph<Vertex>::removeEdge(v, w);
		_c.erase(Edge<Vertex>(v, w));
		_c.erase(Edge<Vertex>(w, v));
	}

	// removes edge if in graph
	void removeEdge(const Edge<Vertex> &e) {
		graph<Vertex>::removeEdge(e.v, e.w);
		_c.erase(e);
		_c.erase(e.reverse());
	}

	// return cost of edge
	double cost(const Vertex &v, const Vertex &w) const {
		assert(graph<Vertex>::isVertex(v) && graph<Vertex>::isVertex(w));
		return _c.at(Edge<Vertex>(v, w));
	}

	// return set of all edges
	std::set< WEdge<Vertex> > E() const {
		std::set< WEdge<Vertex> > ans;
		for (auto &v : graph<Vertex>::V()) {
			for (auto &w : graph<Vertex>::Adj(v)) {
				ans.insert(WEdge<Vertex>(v, w, cost(v,w)));
			}
		}
		return ans;
	}

	// return the minimum spanning wgraph using Kruskal's MST Algorithm
	wgraph<Vertex> Kruskal_MST() const {
		assert(graph<Vertex>::isConnected());
		wgraph<Vertex> ans;

		ds<Vertex> d;

		for (auto &v : graph<Vertex>::V()) {
			ans.addVertex(v);
			d.make_set(v);
		}

		for (auto &e : E()) {
			if (d.join_sets(e.v, e.w)) {
				ans.addEdge(e);
			}
		}

		return ans;
	}

	// return the minimum spanning wgraph using Boruvka's MST Algorithm
	wgraph<Vertex> Boruvka_MST() const {
		assert(graph<Vertex>::isConnected());
		wgraph<Vertex> ans;

		ds<Vertex> d;

		for (auto &v : graph<Vertex>::V()) {
			ans.addVertex(v);
			d.make_set(v);
		}

		while (ans.ncc() > 1) {
			std::map< Vertex, WEdge<Vertex> > lightest;
			
			for (auto &e : E()) {
				if (d.find_set(e.v) != d.find_set(e.w)) {
					Vertex rv = d.root_key(e.v);
					Vertex rw = d.root_key(e.w);
					if (lightest.count(rv) == 0 || lightest.at(rv).c > e.c) {
						lightest[rv] = e;
					}
					if (lightest.count(rw) == 0 || lightest.at(rw).c > e.c) {
						lightest[rw] = e;
					}
				}
			}

			for (auto &p : lightest) {
				WEdge<Vertex> e = p.second;
				ans.addEdge(e);
				d.join_sets(e.v, e.w);
			}
		}

		return ans;
	}

	// return the minimum spanning wgraph using Prim's MST Algorithm
	wgraph<Vertex> Prim_MST() const {
		std::unordered_map<Vertex, double> d;
		Vertex s;
		dary_heap< WEdge<Vertex> > H = dary_heap< WEdge<Vertex> >(std::max((size_t)2, (graph<Vertex>::m()/graph<Vertex>::n())));
		for (auto &v : graph<Vertex>::V()) {
			d[v] = std::numeric_limits<double>::infinity();
			if (d.size() == 0) {
				d[v] = 0;
				s = v;
			}
		}

		std::unordered_map<Vertex, Vertex> parent;
		wgraph<Vertex> ans;
		for (auto &v : graph<Vertex>::V()) {
			H.push(WEdge<Vertex>(v, v, d[v]));
			parent[v] = v;
		}

		while (!H.empty()) {
			WEdge<Vertex> x = H.min();
			H.pop_min();
			ans.addVertex(x.w);
			if (x.w != s) {
				ans.addEdge(x.v, x.w, x.c);
			}
			for (auto &y : graph<Vertex>::Adj(x.w)) {
				double newcost = cost(x.w, y);
				if (!ans.isVertex(y) && newcost < d[y]) {
					H.decrease_key(WEdge<Vertex>(parent.at(y), y, d[y]), WEdge<Vertex>(x.w, y, newcost));
					d[y] = newcost;
					parent[y] = x.w;
				}
			}
		}

		return ans;
	}

private:
	// cost "red-black tree" representation
	std::map<Edge<Vertex>, double> _c;
};

// input wgraph
template <class Vertex>
std::istream& operator >> (std::istream &is, wgraph<Vertex> &W) {
	std::size_t n, m;
	Vertex v, w;
	double c;

	is >> n >> m;
	for (std::size_t i = 0; i < n; i++) {
		is >> v;
		W.addVertex(v);
	}

	for (std::size_t i = 0; i < m; i++) {
		is >> v >> w >> c;
		W.addEdge(v, w, c);
	}

	return is;
}

// output wgraph
template <class Vertex>
std::ostream& operator << (std::ostream &os, const wgraph<Vertex> &W) {
	os << "\nWeighted Graph:" << std::endl;
	os << "# Vertices: " << W.n() << "\n# Edges: " << W.m() << std::endl << std::endl;

	os << "Vertices:";
	for (auto &v : W.V()) {
		os << " " << v;
	}
	os << std::endl << std::endl;

	os << "Edges: " << std::endl;
	for (auto &v : W.V()) {
		for (auto &w : W.Adj(v)) {
			if (v < w) {
				os << v << " " << w << " " << W.cost(v, w) << std::endl;
			}
		}
	}
	os << std::endl;

	return os;
}

#endif // WGRAPH_H
