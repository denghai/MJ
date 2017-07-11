using System;
using System.Collections.Generic;
using System.Linq;
using System.Text;
using System.Threading.Tasks;
using System.Windows.Forms;
namespace TestWinForm
{
    class BetTimesNodeDisplay : addChildPanel
    {
        public override void init(Form11 pForm)
        {
            base.init(pForm);
        }

        public override void refreshCurNodeData(NodeData nCurData)
        {
            base.refreshCurNodeData(nCurData);
            m_panel.enableAddNodeBtn = false;
            m_descLabel.Text = string.Format("当前是{0},\n最终牌型是{1},\n{2} ,\n", nCurData.m_pParentNode.m_pParentNode.m_pParentNode.m_pParentNode.desc,nCurData.m_pParentNode.m_pParentNode.m_pParentNode.desc, nCurData.m_pParentNode.desc);
            NodeBettimes ret = (NodeBettimes)nCurData;
            string strStrage = string.Format("当场上最大下注{0}倍盲注 \n 跟注概率{1}\n 过牌概率{2} \nALL In概率{3}\n 加注概率{4}\n 加注下限{5}倍盲注 \n加注上限{6}倍盲注",
                ret.m_nValue, ret.nFollowRate,ret.nPassRate,ret.nALLInRate,ret.nAddRate,ret.nAddLowLimit,ret.nAddTopLimit);
            m_descLabel.Text = m_descLabel.Text + strStrage;
        }

        public override TreeNode doAddChildNode()
        {
            return null;
        }
    }
}
