#ifndef REFERENCE_H
#define REFERENCE_H

#include "garbagecollector.h"

namespace mint {

class MINT_EXPORT Reference {
public:
	typedef int Flags;
	enum Flag : Flags {
		standard = 0x00,
		const_value = 0x01,
		const_ref = 0x02,
		child_hiden = 0x04,
		user_hiden = 0x08,
		global = 0x10
	};

	Reference(Flags flags = standard, Data *data = Reference::alloc<Data>());
	Reference(const Reference &other);
	~Reference();

	void clone(const Reference &other);
	void copy(const Reference &other);
	void move(const Reference &other);

	Data *data();
	const Data *data() const;

	Flags flags() const;

	template<class T, typename... Args>
	static T *alloc(Args... args) {
		T *data = new T(args...);
		GarbadgeCollector::instance().m_memory[data] = false;
		return data;
	}

	template<class T>
	static Reference *create() {
		Reference *ref = new Reference(const_ref | const_value, alloc<T>());
		return ref;
	}

	template<class Type>
	Type *data() {
		return dynamic_cast<Type *>(data());
	}

	template<class Type>
	const Type *data() const {
		return dynamic_cast<const Type *>(data());
	}

protected:
	static void free(Data *ptr);

private:
	Flags m_flags;
	Data *m_data;

	friend class GarbadgeCollector;
};

class MINT_EXPORT SharedReference {
public:
	SharedReference();
	SharedReference(Reference *ref);
	SharedReference(const SharedReference &other);
	~SharedReference();

	static SharedReference unique(Reference *ref);
	SharedReference &operator =(const SharedReference &other);

	Reference &operator *() const;
	Reference *operator ->() const;
	Reference *get() const;

	bool isUnique() const;

private:
	Reference *m_ref;
	bool m_unique;
};

}

#endif // REFERENCE_H
