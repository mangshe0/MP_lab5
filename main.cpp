#include <iostream>
#include <boost\mpl\vector.hpp>
#include <boost\mpl\int.hpp>
#include <boost\mpl\for_each.hpp>
#include <boost\mpl\transform.hpp>
#include <boost\mpl\filter_view.hpp>
#include <boost\mpl\if.hpp>
#include <boost\mpl\always.hpp>
#include <boost\mpl\begin.hpp>
#include <boost\mpl\end.hpp>
#include <boost\mpl\next_prior.hpp>
#include <boost\mpl\deref.hpp>
#include <boost\mpl\and.hpp>
#include <boost\mpl\not.hpp>
#include <boost\mpl\copy.hpp>
#include <boost\mpl\size.hpp>
#include <boost\mpl\contains.hpp>
namespace mpl = boost::mpl;

struct print_type
{
	template <typename T>
	void operator()(T) const
	{
		std::cout << typeid(T).name() << " ";
	}
};

template <class TSeq>
void print_types()
{
	mpl::for_each<TSeq>(print_type());
	std::cout << std::endl;
}

struct A1 {}; 
struct A2 {}; 
struct A3 {};
struct B1 : public A1 {};
struct B2 : public A2 {};
struct B3 : public A3 {};

typedef mpl::vector<A2, B3, B2, A1, B1, A3>::type input_data;

template <typename curType, typename Base>
struct is_inheritor_of : mpl::and_<
		std::is_base_of<Base, curType>,
		mpl::not_<std::is_same<Base, curType>>
> {};

template <typename Base, typename Begin, typename End>
struct has_inheritor_in
{
	typedef typename mpl::deref<Begin>::type curType;
	typedef typename mpl::next<Begin>::type Next;
	typedef typename mpl::if_<
		is_inheritor_of<curType, Base>,
		mpl::true_,
		typename has_inheritor_in<
			Base,
			Next,
			End
		>::type
	>::type type;
	
};

template <typename X, typename End>
struct has_inheritor_in < X, End, End >
{
	typedef typename mpl::false_ type;
};

struct nil{};

template <typename Base, typename Begin, typename End>
struct get_inheritor_from
{
	typedef typename mpl::deref<Begin>::type curType;
	typedef typename mpl::next<Begin>::type Next;
	typedef typename mpl::if_<
		is_inheritor_of<curType, Base>,
		curType,
		typename get_inheritor_from<
			Base,
			Next,
			End
		>::type
	>::type type;

};

template <typename X, typename End>
struct get_inheritor_from < X, End, End >
{
	typedef nil type;
};

template <typename InputData>
struct splitClassesByInheritance
{
	typedef typename mpl::filter_view <
		typename InputData,
		has_inheritor_in<
			mpl::_1, 
			typename mpl::begin<InputData>::type,
			typename mpl::end<InputData>::type>
	> ::type base_classes;

	typedef typename mpl::copy <
		base_classes,
		mpl::back_inserter<mpl::vector0<>>
	> ::type base_classes_vector;

	typedef typename mpl::transform <
		typename base_classes_vector,
		get_inheritor_from<
			mpl::_1, 
			typename mpl::begin<InputData>::type, 
			typename mpl::end<InputData>::type>
	> ::type inheritors;

	BOOST_STATIC_ASSERT(mpl::size<input_data>::type::value ==
		(mpl::size<base_classes>::type::value + mpl::size<inheritors>::type::value));
};

template <typename InputData>
struct AbstractFactory
{
	typedef typename splitClassesByInheritance<InputData>::base_classes base_classes;
	typedef typename splitClassesByInheritance<InputData>::inheritors inheritors;

	template <typename T>
	T* create();
};

template <typename InputData>
struct ConcreteFactory : public AbstractFactory<InputData>
{
	template <typename T>
	T* create()
	{
		BOOST_STATIC_ASSERT(mpl::contains<inheritors, T>::type::value == true);
		return new T();
	}
};

int main()
{
	print_types<splitClassesByInheritance<input_data>::base_classes>();
	print_types<splitClassesByInheritance<input_data>::inheritors>();

	std::cout << std::endl;
	std::cout << std::endl;

	ConcreteFactory<input_data> factory;

	B1* b1 = factory.create<B1>();
	B2* b2 = factory.create<B2>();
	B3* b3 = factory.create<B3>();

	std::cout << typeid(*b1).name() << std::endl;
	std::cout << typeid(*b2).name() << std::endl;
	std::cout << typeid(*b3).name() << std::endl;

	delete b1;
	delete b2;
	delete b3;
}