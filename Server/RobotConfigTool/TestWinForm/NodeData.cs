using System;
using System.Collections.Generic;
using System.Linq;
using System.Text;
using System.Threading.Tasks;
using System.Collections;
using System.Xml;
namespace TestWinForm
{
    public enum eNodeType
    {
        eNode_Round,
        eNode_CardType,
        eNode_CardValue,
        eNode_ContributeCard,
        eNode_BetTimes,
        eNode_Max,
    }

    enum eCardType
    {
        eCard_HuangJiaTongHuaShun,
        eCard_TongHuaShun,
        eCard_SiTiao,
        eCard_HuLu,
        eCard_TongHua,
        eCard_ShunZi,
        eCard_SanTiao,
        eCard_LiangDui,
        eCard_YiDui,
        eCard_GaoPai,
        eCard_Max,
        eCard_Robot_GaoPai4TongHua = eCard_Max,
        eCard_Robot_GaoPai4ShunZi,
        eCard_Robot_Max,
    };

    class NodeData
    {
        protected eNodeType m_eNodeType;
        public Dictionary<int, NodeData> m_vChildNode;
        public int m_nValue;
        protected string m_nString;
        public NodeData m_pParentNode;
        public static string s_errorString;
        public NodeData(eNodeType eType, int nValue) 
        {
            m_eNodeType = eType; 
            m_nValue = nValue; 
            m_nString = null; 
            m_vChildNode = new Dictionary<int,NodeData>() ;
            m_pParentNode = null;
        }
        public virtual eNodeType getValidChildNodeType() { return eNodeType.eNode_Max;  }
        public virtual bool checkAllValueValid() { return true; }
        public bool checkCanFinish() 
        {
            if (checkAllValueValid() == false)
            {
                s_errorString = string.Format("node type {0} value invalid", m_eNodeType.ToString()); 
                return false;
            }

            if ( m_vChildNode.Count == 0 && m_eNodeType != eNodeType.eNode_BetTimes )
            {
                s_errorString = string.Format("node type {0} , value :{1} have no child", m_eNodeType.ToString(), currentValue);
                return false;
            }

            foreach (NodeData tdata in m_vChildNode.Values)
            {
                if (tdata.checkCanFinish() == false)
                {
                    return false;
                }
            }
            return true;
        }
        public void writeToXmlNode(XmlNode parentNode ,XmlDocument doc )
        {
            XmlElement ele = writeXmlElement(doc); 
            parentNode.AppendChild(ele);
            foreach (NodeData child in m_vChildNode.Values)
            {
                child.writeToXmlNode(ele, doc);
            }
        }

        public virtual bool readFromXmlElement(XmlNode ele)
        {
            return false;
        }

        public virtual XmlElement writeXmlElement(XmlDocument doc)
        {
            XmlElement ele =  doc.CreateElement(m_eNodeType.ToString());
            return ele;
        }

        public eNodeType nodeType
        {
            get { return m_eNodeType;  }
        }

        protected int currentValue
        {
            get { return m_nValue; }
            set { m_nValue = value; }
        }

        public virtual string desc
        {
            get
            {
                return m_nString;
            }
        }

        public virtual string help
        {
            get{ return "no more help";}
        }

        public NodeData addChildNode(NodeData childNode)
        {
            if (getValidChildNodeType() != childNode.nodeType)
            {
                Console.WriteLine("node type = {0} , value = {1} add child = {2}, value {3} type  error ",nodeType,m_nValue,childNode.nodeType,childNode.m_nValue);
                return null;
            }

            if (m_vChildNode.ContainsKey(childNode.m_nValue))
            {
                Console.WriteLine("node type = {0} , value = {1} add child = {2}, value {3} already add  error ", nodeType, m_nValue, childNode.nodeType, childNode.m_nValue);
                return null;
            }

            m_vChildNode.Add(childNode.m_nValue, childNode);
            childNode.m_pParentNode = this;
            return childNode;
        }

        public void removeChildNode(NodeData childNode)
        {
            foreach (int vkey in m_vChildNode.Keys)
            {
                if (vkey == childNode.m_nValue)
                {
                    m_vChildNode.Remove(vkey);
                    return;
                }
            }
        }

        public bool isNodeValueAlreadyExist(int nValue )
        {
            foreach (NodeData tdat in m_vChildNode.Values)
            {
                if ( tdat.m_nValue == nValue)
                {
                    return true;
                }
            }
            return false;
        }
    }

    // round type 
    class NodeRound : NodeData
    {
        public NodeRound(int nRound)
            : base(eNodeType.eNode_Round,nRound)
        {
            m_nString = string.Format("第{0}轮",m_nValue);
        }

        public override string desc
        {
            get
            {
                m_nString = string.Format("第{0}轮", m_nValue);
                switch (m_nValue)
                {
                    case 0:
                        m_nString = m_nString + "----只发了手牌";
                        break;
                    case 1:
                        m_nString = m_nString + "----已经发了3张公共牌";
                        break;
                    case 2:
                        m_nString = m_nString + "----已经发了4张公共牌";
                        break;
                    case 3:
                        m_nString = m_nString + "---5张公共牌已经发完";
                        break;
                    default:
                        m_nString = m_nString + "-----错误了！请debug";
                        break;

                }
                return m_nString;
            }
        }

        public override string help
        {
            get { return "cardType：表示牌型，如同花顺，顺子，两对，三条"; }
        }

        public override eNodeType getValidChildNodeType() { return eNodeType.eNode_CardType; }
        public override bool checkAllValueValid() { return m_nValue >= 0 && m_nValue <= 3; }
        public override XmlElement writeXmlElement(XmlDocument doc)
        {
            XmlElement ele = doc.CreateElement("Round");
            ele.SetAttribute("nCount", m_nValue.ToString());
            return ele;
        }

        public override bool readFromXmlElement(XmlNode pCurNode )
        {
            XmlElement ele = (XmlElement)pCurNode;
            if (ele.Name != "Round")
            {
                s_errorString = "error xml type error for round";
                return false;
            }

            if (ele.HasAttribute("nCount") == false)
            {
                s_errorString = "error xml type do not have nCount attribute";
                return false;
            }

            m_nValue = int.Parse(ele.GetAttribute("nCount"));

            XmlNode pchild = pCurNode.FirstChild;
            while ( pchild != null)
            {
                NodeCardType pnodeChild = new NodeCardType(0);
                pnodeChild.readFromXmlElement(pchild);
                addChildNode(pnodeChild);
                pchild = pchild.NextSibling;
            }
            return true;
        }
    }

    // card type 
    class NodeCardType : NodeData
    {
        public NodeCardType(int eType)
            : base(eNodeType.eNode_CardType,eType)
        {

        }

        public override string desc
        {
            get
            {
                switch ((eCardType)m_nValue)
                {
                    case eCardType.eCard_TongHuaShun:
                        {
                            m_nString = "同花顺";
                        }
                        break;
                    case eCardType.eCard_SiTiao:
                        {
                            m_nString = "四条";
                        }
                        break;
                    case eCardType.eCard_HuLu:
                        {
                            m_nString = "葫芦";
                        }
                        break;
                    case eCardType.eCard_TongHua:
                        {
                            m_nString = "同花";
                        }
                        break;
                    case eCardType.eCard_ShunZi:
                        {
                            m_nString = "顺子";
                        }
                        break;
                    case eCardType.eCard_SanTiao:
                        {
                            m_nString = "三条";
                        }
                        break;
                    case eCardType.eCard_LiangDui:
                        {
                            m_nString = "两对";
                        }
                        break;
                    case eCardType.eCard_YiDui:
                        {
                            m_nString = "一对";
                        }
                        break;
                    case eCardType.eCard_GaoPai:
                        {
                            m_nString = "高牌";
                        }
                        break;
                    case eCardType.eCard_Robot_GaoPai4ShunZi:
                        {
                            m_nString = "高牌4顺子";
                        }
                        break;
                    case eCardType.eCard_Robot_GaoPai4TongHua:
                        {
                            m_nString = "高牌4同花";
                        }
                        break;
                    default:
                        {
                            m_nString = "undefined";
                        }
                        break;
                }
                return m_nString;
            }
        }

        public override string help
        {
            get { return "KeyCard: faceValue表示当前牌型的最终牌型中，玩家手牌起到最大作用的牌（如：手牌为A，8. 最终牌型是 8，8，a，3，2. 此时faceValue = 8. \n因为这个牌型中8的作用更大），如果为0，说明最终牌型里面，没有玩家的手牌。"; }
        }

        public override eNodeType getValidChildNodeType() { return eNodeType.eNode_CardValue; }
        public override bool checkAllValueValid() { return m_nValue >= 1 && m_nValue < (int)eCardType.eCard_Robot_Max; }
        public override XmlElement writeXmlElement(XmlDocument doc)
        {
            XmlElement ele = doc.CreateElement("CardType");
            ele.SetAttribute("caType", m_nValue.ToString());
            return ele;
        }

        public override bool readFromXmlElement(XmlNode pCurNode)
        {
            XmlElement ele = (XmlElement)pCurNode;
            if (ele.Name != "CardType")
            {
                s_errorString = "error xml type error for CardType";
                return false;
            }

            if (ele.HasAttribute("caType") == false)
            {
                s_errorString = "error xml type CardType do not have caType attribute";
                return false;
            }

            m_nValue = int.Parse(ele.GetAttribute("caType"));

            XmlNode pchild = pCurNode.FirstChild;
            while (pchild != null)
            {
                NodeCardValue pnodeChild = new NodeCardValue(0);
                pnodeChild.readFromXmlElement(pchild);
                addChildNode(pnodeChild);
                pchild = pchild.NextSibling;
            }
            return true;
        }

    }

    // face value Type 
    class NodeCardValue : NodeData
    {
        public NodeCardValue(int nValue)
            : base(eNodeType.eNode_CardValue,nValue)
        {

        }

        public override string desc
        {
            get
            {
                string strValue = m_nValue.ToString();
                switch ( m_nValue )
                {
                    case 11: strValue = "J"; break;
                    case 12: strValue = "Q"; break;
                    case 13: strValue = "K"; break;
                    case 14: strValue = "A"; break;
                    default: break;
                }

                if (m_pParentNode == null)
                {
                    m_nString = string.Format("no parent {0}", strValue);
                }
                else
                {
                    m_nString = m_pParentNode.desc + strValue;
                }

                return m_nString;
            }
        }

        public override string help
        {
            get { return "Contribute： 最终牌型里面手牌贡献的牌数量，越多优势越大,当贡献牌 = 0 ，faceValue中表示的手牌提供的非关键牌，\n（如：手牌为3，A. 最终牌型是 8，8，a，3，2. 此时faceValue = A. 因为这个牌型中手牌贡献为0，A 是最大贡献牌）。"; }
        }

        public override eNodeType getValidChildNodeType() { return eNodeType.eNode_ContributeCard; }
        public override bool checkAllValueValid() { return m_nValue >= 2 && m_nValue <= 14; }
        public override XmlElement writeXmlElement(XmlDocument doc)
        {
            XmlElement ele = doc.CreateElement("KeyCard");
            ele.SetAttribute("faceValue", m_nValue.ToString());
            return ele;
        }

        public override bool readFromXmlElement(XmlNode pCurNode)
        {
            XmlElement ele = (XmlElement)pCurNode;
            if (ele.Name != "KeyCard")
            {
                s_errorString = "error xml type error for KeyCard";
                return false;
            }

            if (ele.HasAttribute("faceValue") == false)
            {
                s_errorString = "error xml type KeyCard do not have faceValue attribute";
                return false;
            }

            m_nValue = int.Parse(ele.GetAttribute("faceValue"));

            XmlNode pchild = pCurNode.FirstChild;
            while (pchild != null)
            {
                NodeContribute pnodeChild = new NodeContribute(0);
                pnodeChild.readFromXmlElement(pchild);
                addChildNode(pnodeChild);
                pchild = pchild.NextSibling;
            }
            return true;
        }
    }

    // contribute type 
    class NodeContribute : NodeData
    {
        public NodeContribute(int nValue)
            : base(eNodeType.eNode_ContributeCard,nValue)
        {
            m_nString = string.Format("手牌贡献{0}张",m_nValue);
        }

        public override string desc
        {
            get
            {
                m_nString = string.Format("手牌贡献{0}张", m_nValue);
                return m_nString;
            }
        }

        public override string help
        {
            get { return "BetTimes： 表示当前最大下注额是大盲注的倍数，倍数越高，说明大家拼的情绪比较高。addLowLimit， addTopLimit： 当时加注的时候，在最低加注额之上，上浮多少个盲注倍数。"; }
        }

        public override eNodeType getValidChildNodeType() { return eNodeType.eNode_BetTimes; }
        public override bool checkAllValueValid() { return m_nValue >= 0 && m_nValue <= 2; }
        public override XmlElement writeXmlElement(XmlDocument doc)
        {
            XmlElement ele = doc.CreateElement("HoldCardContribute");
            ele.SetAttribute("count", m_nValue.ToString());
            return ele;
        }

        public override bool readFromXmlElement(XmlNode pCurNode)
        {
            XmlElement ele = (XmlElement)pCurNode;
            if (ele.Name != "HoldCardContribute")
            {
                s_errorString = "error xml type error for HoldCardContribute";
                return false;
            }

            if (ele.HasAttribute("count") == false)
            {
                s_errorString = "error xml type HoldCardContribute do not have count attribute";
                return false;
            }

            m_nValue = int.Parse(ele.GetAttribute("count"));

            XmlNode pchild = pCurNode.FirstChild;
            while (pchild != null)
            {
                NodeBettimes pnodeChild = new NodeBettimes(0);
                pnodeChild.readFromXmlElement(pchild);
                addChildNode(pnodeChild);
                pchild = pchild.NextSibling;
            }
            return true;
        }
    }

    // bet times 
    class NodeBettimes : NodeData
    {
        public int nFollowRate = 0 , nPassRate = 0, nAddRate = 0,nALLInRate = 0, nAddLowLimit = 0 , nAddTopLimit = 0 ;
        public NodeBettimes(int betTimes)
            : base(eNodeType.eNode_BetTimes, betTimes)
        {
            nFollowRate = 0;
            nPassRate = 100;
            nAddRate = 0;
            nALLInRate = 0;
            nAddLowLimit = 1;
            nAddTopLimit = 10;
            m_nString = string.Format("下注倍数{0}", m_nValue);
        }

        public override string desc
        {
            get
            {
                m_nString = string.Format("下注倍数{0}", m_nValue);
                return m_nString;
            }
        }
        public override string help
        {
            get { return "这里是最终的执行策略"; }
        }

        public override bool checkAllValueValid() { return (nFollowRate + nPassRate + nAddRate + nALLInRate) == 100; }
        public override XmlElement writeXmlElement(XmlDocument doc)
        {
            XmlElement ele = doc.CreateElement("BetTimes");
            ele.SetAttribute("nTimes", m_nValue.ToString());

            ele.SetAttribute("Follow", nFollowRate.ToString());
            ele.SetAttribute("Pass", nPassRate.ToString());
            ele.SetAttribute("ALLIn", nALLInRate.ToString());
            ele.SetAttribute("Add", nAddRate.ToString());
            ele.SetAttribute("addLowLimit", nAddLowLimit.ToString());
            ele.SetAttribute("addTopLimit", nAddTopLimit.ToString());

            return ele;
        }

        public override bool readFromXmlElement(XmlNode pCurNode)
        {
            XmlElement ele = (XmlElement)pCurNode;
            if (ele.Name != "BetTimes")
            {
                s_errorString = "error xml type error for BetTimes";
                return false;
            }

            if (ele.HasAttribute("nTimes") == false)
            {
                s_errorString = "error xml type BetTimes do not have nTimes attribute";
                return false;
            }

            m_nValue = int.Parse(ele.GetAttribute("nTimes"));

            if (ele.HasAttribute("Follow") == false)
            {
                s_errorString = "error xml type BetTimes do not have Follow attribute";
                return false;
            }
            nFollowRate = int.Parse(ele.GetAttribute("Follow"));

            if (ele.HasAttribute("Pass") == false)
            {
                s_errorString = "error xml type BetTimes do not have Pass attribute";
                return false;
            }
            nPassRate = int.Parse(ele.GetAttribute("Pass"));

            if (ele.HasAttribute("ALLIn") == false)
            {
                s_errorString = "error xml type BetTimes do not have ALLIn attribute";
                return false;
            }
            nALLInRate = int.Parse(ele.GetAttribute("ALLIn"));

            if (ele.HasAttribute("Add") == false)
            {
                s_errorString = "error xml type BetTimes do not have nTimes attribute";
                return false;
            }
            nAddRate = int.Parse(ele.GetAttribute("Add"));

            if (ele.HasAttribute("addLowLimit") == false)
            {
                s_errorString = "error xml type BetTimes do not have addLowLimit attribute";
                return false;
            }
            nAddLowLimit = int.Parse(ele.GetAttribute("addLowLimit"));

            if (ele.HasAttribute("addLowLimit") == false)
            {
                s_errorString = "error xml type BetTimes do not have addLowLimit attribute";
                return false;
            }
            nAddTopLimit = int.Parse(ele.GetAttribute("addLowLimit"));

            return true;
        }
    }
}
