#ifndef ABSTRACTFACTORY_H
#define ABSTRACTFACTORY_H
#include <unordered_map>
#include <string>
#include <iostream>
namespace AbstractProductTool{
    template<class AbstractProduct>
    class AbstractProductTool{
    public:
        virtual AbstractProduct* createProduct() = 0;
    protected:
        AbstractProductTool(){}
        ~AbstractProductTool(){}
    };
    template<class AbstractProduct>
    class ProductFactory{
    public:
        //单例模式，返回工厂
        static ProductFactory<AbstractProduct> &instance(){
            static ProductFactory<AbstractProduct> factory;
            return factory;
        }
        //注册产品（实际上就是写入hash表的过程）
        void registerProduct(const std::string &name, AbstractProductTool<AbstractProduct>* adsProductTool){
            m_registerProductMap[name] = adsProductTool;
            std::cout<<"name = "<<name<<std::endl;
        }
        //制造产品
        AbstractProduct* createProduct(const std::string &name){
            if (m_registerProductMap.find(name) != m_registerProductMap.end())
			    return m_registerProductMap[name]->createProduct();
		    else
			    return nullptr;
        }
    private:
        ProductFactory(){}
        //工厂对象不可构造
        ProductFactory(const ProductFactory&){}
        //工厂对象不可赋值
        ProductFactory& operator=(const ProductFactory&){}
        //map string--> AbstractProduct
        std::unordered_map<std::string, AbstractProductTool<AbstractProduct> *> m_registerProductMap;
    };
    //具体产品工具类
    template<class AbstractProduct, class ConcreteProduct>
    class ConcreteProductTool :public AbstractProductTool<AbstractProduct>{
    public:
	    explicit ConcreteProductTool(const std::string& name) {
		    ProductFactory<AbstractProduct>::instance().registerProduct(name, this);
	    }

	    AbstractProduct* createProduct() override {
		    return new ConcreteProduct();
	    }
    };
}

//利用全局静态变量的初始化特性实现产品类的自注册
#define RegisterProduct(AbstractProduct, ConcreteProduct) \
	static AbstractProductTool::ConcreteProductTool<AbstractProduct, ConcreteProduct> register##ConcreteProduct(#ConcreteProduct);
#endif