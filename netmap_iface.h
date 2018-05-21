
#ifndef NETMAP_IFACE_H
#define NETMAP_IFACE_H

#define NETMAP_WITH_LIBS
#include <net/netmap_user.h>

#include <string>
#include <vector>
#include <stdexcept>

class netmap_iface
{
	class _ring_proxy
	{
	public:
		enum class dir { rx = 0, tx = 1 };

		explicit _ring_proxy(const netmap_iface& iface_, dir dir_)
			: _dir(dir_),
			  _iface(iface_) { }

		netmap_ring* operator[](unsigned i) const
		{
			switch (_dir) {
				case dir::rx:
					if (i >= _iface.count_rx_rings())
						throw std::logic_error("netmap_iface: invalid rx ring id " + i);
					return NETMAP_RXRING(_iface._nmd->nifp, i);
				case dir::tx:
					if (i >= _iface.count_tx_rings())
						throw std::logic_error("netmap_iface: invalid tx ring id " + i);
					return NETMAP_TXRING(_iface._nmd->nifp, i);
			}
		}

	private:
		dir _dir;
		const netmap_iface& _iface;
	};

public:

	explicit netmap_iface(const std::string& iface_name_)
		: _nmd(_open(iface_name_)),
		  tx_rings(*this, _ring_proxy::dir::tx),
		  rx_rings(*this, _ring_proxy::dir::rx) { }

	inline unsigned count_rx_rings() const
	{
		return _nmd->req.nr_rx_rings;
	}

	inline unsigned count_tx_rings() const
	{
		return _nmd->req.nr_tx_rings;
	}

	inline int fd() const
	{
		return _nmd->fd;
	}

	~netmap_iface()
	{
		nm_close(_nmd);
	}

	_ring_proxy tx_rings;
	_ring_proxy rx_rings;

private:

	struct nm_desc* _open(const std::string& iface_name_)
	{
		struct nm_desc base_nmd { };
		bzero(&base_nmd, sizeof(base_nmd));
		struct nm_desc* nmd = nm_open(("netmap:" + iface_name_).c_str(), nullptr, 0, &base_nmd);

		if (!nmd)
			throw std::runtime_error("netmap_iface: could not open device " + iface_name_);

		struct netmap_if* nifp = nmd->nifp;
		struct nmreq* req = &nmd->req;

		return nmd;
	}

	struct nm_desc* _nmd = nullptr;
};

#endif
