namespace TestWinForm
{
    partial class Form11
    {
        /// <summary>
        /// 必需的设计器变量。
        /// </summary>
        private System.ComponentModel.IContainer components = null;

        /// <summary>
        /// 清理所有正在使用的资源。
        /// </summary>
        /// <param name="disposing">如果应释放托管资源，为 true；否则为 false。</param>
        protected override void Dispose(bool disposing)
        {
            if (disposing && (components != null))
            {
                components.Dispose();
            }
            base.Dispose(disposing);
        }

        #region Windows 窗体设计器生成的代码

        /// <summary>
        /// 设计器支持所需的方法 - 不要
        /// 使用代码编辑器修改此方法的内容。
        /// </summary>
        private void InitializeComponent()
        {
            this.treeView1 = new System.Windows.Forms.TreeView();
            this.addcardtype = new System.Windows.Forms.Panel();
            this.tonghuashun = new System.Windows.Forms.RadioButton();
            this.tonghua = new System.Windows.Forms.RadioButton();
            this.shunzi = new System.Windows.Forms.RadioButton();
            this.liangdui = new System.Windows.Forms.RadioButton();
            this.santiao = new System.Windows.Forms.RadioButton();
            this.hulu = new System.Windows.Forms.RadioButton();
            this.sitiao = new System.Windows.Forms.RadioButton();
            this.yidui = new System.Windows.Forms.RadioButton();
            this.gaopai = new System.Windows.Forms.RadioButton();
            this.addcardfacevlue = new System.Windows.Forms.Panel();
            this.v3 = new System.Windows.Forms.RadioButton();
            this.v4 = new System.Windows.Forms.RadioButton();
            this.v5 = new System.Windows.Forms.RadioButton();
            this.v6 = new System.Windows.Forms.RadioButton();
            this.v7 = new System.Windows.Forms.RadioButton();
            this.v8 = new System.Windows.Forms.RadioButton();
            this.v9 = new System.Windows.Forms.RadioButton();
            this.v10 = new System.Windows.Forms.RadioButton();
            this.v11 = new System.Windows.Forms.RadioButton();
            this.v12 = new System.Windows.Forms.RadioButton();
            this.v13 = new System.Windows.Forms.RadioButton();
            this.v14 = new System.Windows.Forms.RadioButton();
            this.v2 = new System.Windows.Forms.RadioButton();
            this.addNodeBtn = new System.Windows.Forms.Button();
            this.desclabel = new System.Windows.Forms.Label();
            this.btnRemoveNode = new System.Windows.Forms.Button();
            this.addcontribute = new System.Windows.Forms.Panel();
            this.con2 = new System.Windows.Forms.RadioButton();
            this.con1 = new System.Windows.Forms.RadioButton();
            this.con0 = new System.Windows.Forms.RadioButton();
            this.addbettimes = new System.Windows.Forms.Panel();
            this.button3 = new System.Windows.Forms.Button();
            this.leftWeight = new System.Windows.Forms.Label();
            this.button1 = new System.Windows.Forms.Button();
            this.pass = new System.Windows.Forms.NumericUpDown();
            this.follow = new System.Windows.Forms.NumericUpDown();
            this.all = new System.Windows.Forms.NumericUpDown();
            this.add = new System.Windows.Forms.NumericUpDown();
            this.addlowlimit = new System.Windows.Forms.NumericUpDown();
            this.addtoplimit = new System.Windows.Forms.NumericUpDown();
            this.bettimes = new System.Windows.Forms.NumericUpDown();
            this.label8 = new System.Windows.Forms.Label();
            this.label7 = new System.Windows.Forms.Label();
            this.label6 = new System.Windows.Forms.Label();
            this.label5 = new System.Windows.Forms.Label();
            this.label4 = new System.Windows.Forms.Label();
            this.label3 = new System.Windows.Forms.Label();
            this.label2 = new System.Windows.Forms.Label();
            this.menuStrip1 = new System.Windows.Forms.MenuStrip();
            this.新建文档ToolStripMenuItem = new System.Windows.Forms.ToolStripMenuItem();
            this.新建文档ToolStripMenuItem1 = new System.Windows.Forms.ToolStripMenuItem();
            this.保存ToolStripMenuItem = new System.Windows.Forms.ToolStripMenuItem();
            this.另存为ToolStripMenuItem = new System.Windows.Forms.ToolStripMenuItem();
            this.打开文件ToolStripMenuItem = new System.Windows.Forms.ToolStripMenuItem();
            this.gaopai4tonghua = new System.Windows.Forms.RadioButton();
            this.gaopai4shunzi = new System.Windows.Forms.RadioButton();
            this.infotext = new System.Windows.Forms.Label();
            this.v0 = new System.Windows.Forms.RadioButton();
            this.帮助ToolStripMenuItem = new System.Windows.Forms.ToolStripMenuItem();
            this.addcardtype.SuspendLayout();
            this.addcardfacevlue.SuspendLayout();
            this.addcontribute.SuspendLayout();
            this.addbettimes.SuspendLayout();
            ((System.ComponentModel.ISupportInitialize)(this.pass)).BeginInit();
            ((System.ComponentModel.ISupportInitialize)(this.follow)).BeginInit();
            ((System.ComponentModel.ISupportInitialize)(this.all)).BeginInit();
            ((System.ComponentModel.ISupportInitialize)(this.add)).BeginInit();
            ((System.ComponentModel.ISupportInitialize)(this.addlowlimit)).BeginInit();
            ((System.ComponentModel.ISupportInitialize)(this.addtoplimit)).BeginInit();
            ((System.ComponentModel.ISupportInitialize)(this.bettimes)).BeginInit();
            this.menuStrip1.SuspendLayout();
            this.SuspendLayout();
            // 
            // treeView1
            // 
            this.treeView1.Location = new System.Drawing.Point(24, 35);
            this.treeView1.Name = "treeView1";
            this.treeView1.Size = new System.Drawing.Size(272, 426);
            this.treeView1.TabIndex = 0;
            this.treeView1.BeforeSelect += new System.Windows.Forms.TreeViewCancelEventHandler(this.treeView1_BeforeSelect);
            this.treeView1.AfterSelect += new System.Windows.Forms.TreeViewEventHandler(this.treeView1_AfterSelect);
            // 
            // addcardtype
            // 
            this.addcardtype.Anchor = ((System.Windows.Forms.AnchorStyles)(((System.Windows.Forms.AnchorStyles.Top | System.Windows.Forms.AnchorStyles.Left) 
            | System.Windows.Forms.AnchorStyles.Right)));
            this.addcardtype.Controls.Add(this.gaopai4shunzi);
            this.addcardtype.Controls.Add(this.gaopai4tonghua);
            this.addcardtype.Controls.Add(this.tonghuashun);
            this.addcardtype.Controls.Add(this.tonghua);
            this.addcardtype.Controls.Add(this.shunzi);
            this.addcardtype.Controls.Add(this.liangdui);
            this.addcardtype.Controls.Add(this.santiao);
            this.addcardtype.Controls.Add(this.hulu);
            this.addcardtype.Controls.Add(this.sitiao);
            this.addcardtype.Controls.Add(this.yidui);
            this.addcardtype.Controls.Add(this.gaopai);
            this.addcardtype.Location = new System.Drawing.Point(430, 71);
            this.addcardtype.Name = "addcardtype";
            this.addcardtype.Size = new System.Drawing.Size(362, 217);
            this.addcardtype.TabIndex = 1;
            // 
            // tonghuashun
            // 
            this.tonghuashun.AutoSize = true;
            this.tonghuashun.Location = new System.Drawing.Point(236, 49);
            this.tonghuashun.Name = "tonghuashun";
            this.tonghuashun.Size = new System.Drawing.Size(59, 16);
            this.tonghuashun.TabIndex = 9;
            this.tonghuashun.TabStop = true;
            this.tonghuashun.Text = "同花顺";
            this.tonghuashun.UseVisualStyleBackColor = true;
            // 
            // tonghua
            // 
            this.tonghua.AutoSize = true;
            this.tonghua.Location = new System.Drawing.Point(163, 146);
            this.tonghua.Name = "tonghua";
            this.tonghua.Size = new System.Drawing.Size(47, 16);
            this.tonghua.TabIndex = 8;
            this.tonghua.TabStop = true;
            this.tonghua.Text = "同花";
            this.tonghua.UseVisualStyleBackColor = true;
            // 
            // shunzi
            // 
            this.shunzi.AutoSize = true;
            this.shunzi.Location = new System.Drawing.Point(163, 115);
            this.shunzi.Name = "shunzi";
            this.shunzi.Size = new System.Drawing.Size(47, 16);
            this.shunzi.TabIndex = 7;
            this.shunzi.TabStop = true;
            this.shunzi.Text = "顺子";
            this.shunzi.UseVisualStyleBackColor = true;
            // 
            // liangdui
            // 
            this.liangdui.AutoSize = true;
            this.liangdui.Location = new System.Drawing.Point(51, 115);
            this.liangdui.Name = "liangdui";
            this.liangdui.Size = new System.Drawing.Size(47, 16);
            this.liangdui.TabIndex = 6;
            this.liangdui.TabStop = true;
            this.liangdui.Text = "两对";
            this.liangdui.UseVisualStyleBackColor = true;
            // 
            // santiao
            // 
            this.santiao.AutoSize = true;
            this.santiao.Location = new System.Drawing.Point(51, 146);
            this.santiao.Name = "santiao";
            this.santiao.Size = new System.Drawing.Size(47, 16);
            this.santiao.TabIndex = 5;
            this.santiao.TabStop = true;
            this.santiao.Text = "三条";
            this.santiao.UseVisualStyleBackColor = true;
            // 
            // hulu
            // 
            this.hulu.AutoSize = true;
            this.hulu.Location = new System.Drawing.Point(163, 49);
            this.hulu.Name = "hulu";
            this.hulu.Size = new System.Drawing.Size(47, 16);
            this.hulu.TabIndex = 4;
            this.hulu.TabStop = true;
            this.hulu.Text = "葫芦";
            this.hulu.UseVisualStyleBackColor = true;
            // 
            // sitiao
            // 
            this.sitiao.AutoSize = true;
            this.sitiao.Location = new System.Drawing.Point(163, 82);
            this.sitiao.Name = "sitiao";
            this.sitiao.Size = new System.Drawing.Size(47, 16);
            this.sitiao.TabIndex = 3;
            this.sitiao.TabStop = true;
            this.sitiao.Text = "四条";
            this.sitiao.UseVisualStyleBackColor = true;
            // 
            // yidui
            // 
            this.yidui.AutoSize = true;
            this.yidui.Location = new System.Drawing.Point(51, 82);
            this.yidui.Name = "yidui";
            this.yidui.Size = new System.Drawing.Size(47, 16);
            this.yidui.TabIndex = 2;
            this.yidui.TabStop = true;
            this.yidui.Text = "一对";
            this.yidui.UseVisualStyleBackColor = true;
            // 
            // gaopai
            // 
            this.gaopai.AutoSize = true;
            this.gaopai.Location = new System.Drawing.Point(51, 49);
            this.gaopai.Name = "gaopai";
            this.gaopai.Size = new System.Drawing.Size(47, 16);
            this.gaopai.TabIndex = 1;
            this.gaopai.TabStop = true;
            this.gaopai.Text = "高牌";
            this.gaopai.UseVisualStyleBackColor = true;
            // 
            // addcardfacevlue
            // 
            this.addcardfacevlue.Anchor = ((System.Windows.Forms.AnchorStyles)(((System.Windows.Forms.AnchorStyles.Top | System.Windows.Forms.AnchorStyles.Left) 
            | System.Windows.Forms.AnchorStyles.Right)));
            this.addcardfacevlue.Controls.Add(this.v0);
            this.addcardfacevlue.Controls.Add(this.v3);
            this.addcardfacevlue.Controls.Add(this.v4);
            this.addcardfacevlue.Controls.Add(this.v5);
            this.addcardfacevlue.Controls.Add(this.v6);
            this.addcardfacevlue.Controls.Add(this.v7);
            this.addcardfacevlue.Controls.Add(this.v8);
            this.addcardfacevlue.Controls.Add(this.v9);
            this.addcardfacevlue.Controls.Add(this.v10);
            this.addcardfacevlue.Controls.Add(this.v11);
            this.addcardfacevlue.Controls.Add(this.v12);
            this.addcardfacevlue.Controls.Add(this.v13);
            this.addcardfacevlue.Controls.Add(this.v14);
            this.addcardfacevlue.Controls.Add(this.v2);
            this.addcardfacevlue.Location = new System.Drawing.Point(761, 28);
            this.addcardfacevlue.Name = "addcardfacevlue";
            this.addcardfacevlue.Size = new System.Drawing.Size(356, 170);
            this.addcardfacevlue.TabIndex = 2;
            // 
            // v3
            // 
            this.v3.AutoSize = true;
            this.v3.Location = new System.Drawing.Point(48, 66);
            this.v3.Name = "v3";
            this.v3.Size = new System.Drawing.Size(29, 16);
            this.v3.TabIndex = 13;
            this.v3.TabStop = true;
            this.v3.Text = "3";
            this.v3.UseVisualStyleBackColor = true;
            // 
            // v4
            // 
            this.v4.AutoSize = true;
            this.v4.Location = new System.Drawing.Point(47, 97);
            this.v4.Name = "v4";
            this.v4.Size = new System.Drawing.Size(29, 16);
            this.v4.TabIndex = 12;
            this.v4.TabStop = true;
            this.v4.Text = "4";
            this.v4.UseVisualStyleBackColor = true;
            // 
            // v5
            // 
            this.v5.AutoSize = true;
            this.v5.Location = new System.Drawing.Point(47, 130);
            this.v5.Name = "v5";
            this.v5.Size = new System.Drawing.Size(29, 16);
            this.v5.TabIndex = 11;
            this.v5.TabStop = true;
            this.v5.Text = "5";
            this.v5.UseVisualStyleBackColor = true;
            // 
            // v6
            // 
            this.v6.AutoSize = true;
            this.v6.Location = new System.Drawing.Point(133, 36);
            this.v6.Name = "v6";
            this.v6.Size = new System.Drawing.Size(29, 16);
            this.v6.TabIndex = 10;
            this.v6.TabStop = true;
            this.v6.Text = "6";
            this.v6.UseVisualStyleBackColor = true;
            // 
            // v7
            // 
            this.v7.AutoSize = true;
            this.v7.Location = new System.Drawing.Point(133, 66);
            this.v7.Name = "v7";
            this.v7.Size = new System.Drawing.Size(29, 16);
            this.v7.TabIndex = 9;
            this.v7.TabStop = true;
            this.v7.Text = "7";
            this.v7.UseVisualStyleBackColor = true;
            // 
            // v8
            // 
            this.v8.AutoSize = true;
            this.v8.Location = new System.Drawing.Point(133, 97);
            this.v8.Name = "v8";
            this.v8.Size = new System.Drawing.Size(29, 16);
            this.v8.TabIndex = 8;
            this.v8.TabStop = true;
            this.v8.Text = "8";
            this.v8.UseVisualStyleBackColor = true;
            // 
            // v9
            // 
            this.v9.AutoSize = true;
            this.v9.Location = new System.Drawing.Point(133, 130);
            this.v9.Name = "v9";
            this.v9.Size = new System.Drawing.Size(29, 16);
            this.v9.TabIndex = 7;
            this.v9.TabStop = true;
            this.v9.Text = "9";
            this.v9.UseVisualStyleBackColor = true;
            // 
            // v10
            // 
            this.v10.AutoSize = true;
            this.v10.Location = new System.Drawing.Point(224, 36);
            this.v10.Name = "v10";
            this.v10.Size = new System.Drawing.Size(35, 16);
            this.v10.TabIndex = 6;
            this.v10.TabStop = true;
            this.v10.Text = "10";
            this.v10.UseVisualStyleBackColor = true;
            // 
            // v11
            // 
            this.v11.AutoSize = true;
            this.v11.Location = new System.Drawing.Point(224, 66);
            this.v11.Name = "v11";
            this.v11.Size = new System.Drawing.Size(29, 16);
            this.v11.TabIndex = 5;
            this.v11.TabStop = true;
            this.v11.Text = "J";
            this.v11.UseVisualStyleBackColor = true;
            // 
            // v12
            // 
            this.v12.AutoSize = true;
            this.v12.Location = new System.Drawing.Point(224, 97);
            this.v12.Name = "v12";
            this.v12.Size = new System.Drawing.Size(29, 16);
            this.v12.TabIndex = 4;
            this.v12.TabStop = true;
            this.v12.Text = "Q";
            this.v12.UseVisualStyleBackColor = true;
            // 
            // v13
            // 
            this.v13.AutoSize = true;
            this.v13.Location = new System.Drawing.Point(224, 130);
            this.v13.Name = "v13";
            this.v13.Size = new System.Drawing.Size(29, 16);
            this.v13.TabIndex = 3;
            this.v13.TabStop = true;
            this.v13.Text = "K";
            this.v13.UseVisualStyleBackColor = true;
            // 
            // v14
            // 
            this.v14.AutoSize = true;
            this.v14.Location = new System.Drawing.Point(282, 66);
            this.v14.Name = "v14";
            this.v14.Size = new System.Drawing.Size(29, 16);
            this.v14.TabIndex = 2;
            this.v14.TabStop = true;
            this.v14.Text = "A";
            this.v14.UseVisualStyleBackColor = true;
            // 
            // v2
            // 
            this.v2.AutoSize = true;
            this.v2.Location = new System.Drawing.Point(48, 36);
            this.v2.Name = "v2";
            this.v2.Size = new System.Drawing.Size(29, 16);
            this.v2.TabIndex = 0;
            this.v2.TabStop = true;
            this.v2.Text = "2";
            this.v2.UseVisualStyleBackColor = true;
            // 
            // addNodeBtn
            // 
            this.addNodeBtn.Location = new System.Drawing.Point(632, 393);
            this.addNodeBtn.Name = "addNodeBtn";
            this.addNodeBtn.Size = new System.Drawing.Size(104, 23);
            this.addNodeBtn.TabIndex = 10;
            this.addNodeBtn.Text = "添加牌型节点";
            this.addNodeBtn.UseVisualStyleBackColor = true;
            this.addNodeBtn.Click += new System.EventHandler(this.addNodeBtnClick);
            // 
            // desclabel
            // 
            this.desclabel.Anchor = ((System.Windows.Forms.AnchorStyles)(((System.Windows.Forms.AnchorStyles.Top | System.Windows.Forms.AnchorStyles.Left) 
            | System.Windows.Forms.AnchorStyles.Right)));
            this.desclabel.AutoSize = true;
            this.desclabel.Font = new System.Drawing.Font("宋体", 13F);
            this.desclabel.ForeColor = System.Drawing.Color.FromArgb(((int)(((byte)(192)))), ((int)(((byte)(0)))), ((int)(((byte)(0)))));
            this.desclabel.Location = new System.Drawing.Point(430, 35);
            this.desclabel.Name = "desclabel";
            this.desclabel.Size = new System.Drawing.Size(116, 18);
            this.desclabel.TabIndex = 3;
            this.desclabel.Text = "当前节点介绍";
            this.desclabel.TextAlign = System.Drawing.ContentAlignment.TopCenter;
            // 
            // btnRemoveNode
            // 
            this.btnRemoveNode.Location = new System.Drawing.Point(481, 393);
            this.btnRemoveNode.Name = "btnRemoveNode";
            this.btnRemoveNode.Size = new System.Drawing.Size(117, 23);
            this.btnRemoveNode.TabIndex = 11;
            this.btnRemoveNode.Text = "删除当前节点";
            this.btnRemoveNode.UseVisualStyleBackColor = true;
            this.btnRemoveNode.Click += new System.EventHandler(this.removeNodeBtnClick);
            // 
            // addcontribute
            // 
            this.addcontribute.Controls.Add(this.con2);
            this.addcontribute.Controls.Add(this.con1);
            this.addcontribute.Controls.Add(this.con0);
            this.addcontribute.Location = new System.Drawing.Point(128, 35);
            this.addcontribute.Name = "addcontribute";
            this.addcontribute.Size = new System.Drawing.Size(168, 183);
            this.addcontribute.TabIndex = 12;
            // 
            // con2
            // 
            this.con2.AutoSize = true;
            this.con2.Location = new System.Drawing.Point(24, 127);
            this.con2.Name = "con2";
            this.con2.Size = new System.Drawing.Size(107, 16);
            this.con2.TabIndex = 2;
            this.con2.TabStop = true;
            this.con2.Text = "2 张牌来自手牌";
            this.con2.UseVisualStyleBackColor = true;
            // 
            // con1
            // 
            this.con1.AutoSize = true;
            this.con1.Location = new System.Drawing.Point(24, 90);
            this.con1.Name = "con1";
            this.con1.Size = new System.Drawing.Size(107, 16);
            this.con1.TabIndex = 1;
            this.con1.TabStop = true;
            this.con1.Text = "1 张牌来自手牌";
            this.con1.UseVisualStyleBackColor = true;
            // 
            // con0
            // 
            this.con0.AutoSize = true;
            this.con0.Location = new System.Drawing.Point(24, 48);
            this.con0.Name = "con0";
            this.con0.Size = new System.Drawing.Size(107, 16);
            this.con0.TabIndex = 0;
            this.con0.TabStop = true;
            this.con0.Text = "0 张牌来自手牌";
            this.con0.UseVisualStyleBackColor = true;
            // 
            // addbettimes
            // 
            this.addbettimes.Anchor = ((System.Windows.Forms.AnchorStyles)(((System.Windows.Forms.AnchorStyles.Top | System.Windows.Forms.AnchorStyles.Left) 
            | System.Windows.Forms.AnchorStyles.Right)));
            this.addbettimes.Controls.Add(this.button3);
            this.addbettimes.Controls.Add(this.leftWeight);
            this.addbettimes.Controls.Add(this.button1);
            this.addbettimes.Controls.Add(this.pass);
            this.addbettimes.Controls.Add(this.follow);
            this.addbettimes.Controls.Add(this.all);
            this.addbettimes.Controls.Add(this.add);
            this.addbettimes.Controls.Add(this.addlowlimit);
            this.addbettimes.Controls.Add(this.addtoplimit);
            this.addbettimes.Controls.Add(this.bettimes);
            this.addbettimes.Controls.Add(this.label8);
            this.addbettimes.Controls.Add(this.label7);
            this.addbettimes.Controls.Add(this.label6);
            this.addbettimes.Controls.Add(this.label5);
            this.addbettimes.Controls.Add(this.label4);
            this.addbettimes.Controls.Add(this.label3);
            this.addbettimes.Controls.Add(this.label2);
            this.addbettimes.Location = new System.Drawing.Point(75, 253);
            this.addbettimes.Name = "addbettimes";
            this.addbettimes.Size = new System.Drawing.Size(382, 163);
            this.addbettimes.TabIndex = 13;
            // 
            // button3
            // 
            this.button3.Location = new System.Drawing.Point(290, 8);
            this.button3.Name = "button3";
            this.button3.Size = new System.Drawing.Size(75, 23);
            this.button3.TabIndex = 16;
            this.button3.Text = "均分权重";
            this.button3.UseVisualStyleBackColor = true;
            this.button3.Click += new System.EventHandler(this.equalDivisonClick);
            // 
            // leftWeight
            // 
            this.leftWeight.AutoSize = true;
            this.leftWeight.ForeColor = System.Drawing.Color.Green;
            this.leftWeight.Location = new System.Drawing.Point(173, 12);
            this.leftWeight.Name = "leftWeight";
            this.leftWeight.Size = new System.Drawing.Size(77, 12);
            this.leftWeight.TabIndex = 15;
            this.leftWeight.Text = "剩余权重:100";
            // 
            // button1
            // 
            this.button1.Location = new System.Drawing.Point(21, 134);
            this.button1.Name = "button1";
            this.button1.Size = new System.Drawing.Size(75, 23);
            this.button1.TabIndex = 14;
            this.button1.Text = "重置";
            this.button1.UseVisualStyleBackColor = true;
            this.button1.Click += new System.EventHandler(this.resetWeightClick);
            // 
            // pass
            // 
            this.pass.Location = new System.Drawing.Point(89, 41);
            this.pass.Name = "pass";
            this.pass.Size = new System.Drawing.Size(75, 21);
            this.pass.TabIndex = 13;
            this.pass.ValueChanged += new System.EventHandler(this.strageValueChanged);
            // 
            // follow
            // 
            this.follow.Location = new System.Drawing.Point(89, 74);
            this.follow.Name = "follow";
            this.follow.Size = new System.Drawing.Size(75, 21);
            this.follow.TabIndex = 12;
            this.follow.ValueChanged += new System.EventHandler(this.strageValueChanged);
            // 
            // all
            // 
            this.all.Location = new System.Drawing.Point(89, 107);
            this.all.Name = "all";
            this.all.Size = new System.Drawing.Size(75, 21);
            this.all.TabIndex = 11;
            this.all.ValueChanged += new System.EventHandler(this.strageValueChanged);
            // 
            // add
            // 
            this.add.Location = new System.Drawing.Point(290, 41);
            this.add.Name = "add";
            this.add.Size = new System.Drawing.Size(75, 21);
            this.add.TabIndex = 10;
            this.add.ValueChanged += new System.EventHandler(this.strageValueChanged);
            // 
            // addlowlimit
            // 
            this.addlowlimit.Location = new System.Drawing.Point(290, 74);
            this.addlowlimit.Name = "addlowlimit";
            this.addlowlimit.Size = new System.Drawing.Size(75, 21);
            this.addlowlimit.TabIndex = 9;
            // 
            // addtoplimit
            // 
            this.addtoplimit.Location = new System.Drawing.Point(290, 107);
            this.addtoplimit.Name = "addtoplimit";
            this.addtoplimit.Size = new System.Drawing.Size(75, 21);
            this.addtoplimit.TabIndex = 8;
            // 
            // bettimes
            // 
            this.bettimes.Location = new System.Drawing.Point(89, 10);
            this.bettimes.Name = "bettimes";
            this.bettimes.Size = new System.Drawing.Size(75, 21);
            this.bettimes.TabIndex = 7;
            // 
            // label8
            // 
            this.label8.AutoSize = true;
            this.label8.Font = new System.Drawing.Font("宋体", 10F);
            this.label8.Location = new System.Drawing.Point(18, 114);
            this.label8.Name = "label8";
            this.label8.Size = new System.Drawing.Size(49, 14);
            this.label8.TabIndex = 6;
            this.label8.Text = "All In";
            // 
            // label7
            // 
            this.label7.AutoSize = true;
            this.label7.Font = new System.Drawing.Font("宋体", 10F);
            this.label7.Location = new System.Drawing.Point(210, 48);
            this.label7.Name = "label7";
            this.label7.Size = new System.Drawing.Size(35, 14);
            this.label7.TabIndex = 5;
            this.label7.Text = "加注";
            // 
            // label6
            // 
            this.label6.AutoSize = true;
            this.label6.Font = new System.Drawing.Font("宋体", 10F);
            this.label6.Location = new System.Drawing.Point(199, 81);
            this.label6.Name = "label6";
            this.label6.Size = new System.Drawing.Size(63, 14);
            this.label6.TabIndex = 4;
            this.label6.Text = "加注下限";
            // 
            // label5
            // 
            this.label5.AutoSize = true;
            this.label5.Font = new System.Drawing.Font("宋体", 10F);
            this.label5.Location = new System.Drawing.Point(199, 114);
            this.label5.Name = "label5";
            this.label5.Size = new System.Drawing.Size(63, 14);
            this.label5.TabIndex = 3;
            this.label5.Text = "加注上限";
            // 
            // label4
            // 
            this.label4.AutoSize = true;
            this.label4.Font = new System.Drawing.Font("宋体", 10F);
            this.label4.Location = new System.Drawing.Point(18, 48);
            this.label4.Name = "label4";
            this.label4.Size = new System.Drawing.Size(35, 14);
            this.label4.TabIndex = 2;
            this.label4.Text = "过牌";
            // 
            // label3
            // 
            this.label3.AutoSize = true;
            this.label3.Font = new System.Drawing.Font("宋体", 10F);
            this.label3.Location = new System.Drawing.Point(18, 81);
            this.label3.Name = "label3";
            this.label3.Size = new System.Drawing.Size(35, 14);
            this.label3.TabIndex = 1;
            this.label3.Text = "跟注";
            // 
            // label2
            // 
            this.label2.AutoSize = true;
            this.label2.Font = new System.Drawing.Font("宋体", 10F);
            this.label2.Location = new System.Drawing.Point(7, 15);
            this.label2.Name = "label2";
            this.label2.Size = new System.Drawing.Size(63, 14);
            this.label2.TabIndex = 0;
            this.label2.Text = "下注倍数";
            // 
            // menuStrip1
            // 
            this.menuStrip1.Items.AddRange(new System.Windows.Forms.ToolStripItem[] {
            this.新建文档ToolStripMenuItem,
            this.帮助ToolStripMenuItem});
            this.menuStrip1.Location = new System.Drawing.Point(0, 0);
            this.menuStrip1.Name = "menuStrip1";
            this.menuStrip1.Size = new System.Drawing.Size(1184, 25);
            this.menuStrip1.TabIndex = 14;
            this.menuStrip1.Text = "menuStrip1";
            // 
            // 新建文档ToolStripMenuItem
            // 
            this.新建文档ToolStripMenuItem.DropDownItems.AddRange(new System.Windows.Forms.ToolStripItem[] {
            this.新建文档ToolStripMenuItem1,
            this.保存ToolStripMenuItem,
            this.另存为ToolStripMenuItem,
            this.打开文件ToolStripMenuItem});
            this.新建文档ToolStripMenuItem.Name = "新建文档ToolStripMenuItem";
            this.新建文档ToolStripMenuItem.Size = new System.Drawing.Size(44, 21);
            this.新建文档ToolStripMenuItem.Text = "文件";
            // 
            // 新建文档ToolStripMenuItem1
            // 
            this.新建文档ToolStripMenuItem1.Name = "新建文档ToolStripMenuItem1";
            this.新建文档ToolStripMenuItem1.ShortcutKeys = ((System.Windows.Forms.Keys)((System.Windows.Forms.Keys.Control | System.Windows.Forms.Keys.N)));
            this.新建文档ToolStripMenuItem1.Size = new System.Drawing.Size(171, 22);
            this.新建文档ToolStripMenuItem1.Text = "新建文档";
            this.新建文档ToolStripMenuItem1.Click += new System.EventHandler(this.新建文档ToolStripMenuItem1_Click);
            // 
            // 保存ToolStripMenuItem
            // 
            this.保存ToolStripMenuItem.Name = "保存ToolStripMenuItem";
            this.保存ToolStripMenuItem.ShortcutKeys = ((System.Windows.Forms.Keys)((System.Windows.Forms.Keys.Control | System.Windows.Forms.Keys.S)));
            this.保存ToolStripMenuItem.Size = new System.Drawing.Size(171, 22);
            this.保存ToolStripMenuItem.Text = "保存";
            this.保存ToolStripMenuItem.Click += new System.EventHandler(this.保存ToolStripMenuItem_Click);
            // 
            // 另存为ToolStripMenuItem
            // 
            this.另存为ToolStripMenuItem.Name = "另存为ToolStripMenuItem";
            this.另存为ToolStripMenuItem.ShortcutKeys = ((System.Windows.Forms.Keys)((System.Windows.Forms.Keys.Control | System.Windows.Forms.Keys.R)));
            this.另存为ToolStripMenuItem.Size = new System.Drawing.Size(171, 22);
            this.另存为ToolStripMenuItem.Text = "另存为";
            this.另存为ToolStripMenuItem.Click += new System.EventHandler(this.另存为ToolStripMenuItem_Click);
            // 
            // 打开文件ToolStripMenuItem
            // 
            this.打开文件ToolStripMenuItem.Name = "打开文件ToolStripMenuItem";
            this.打开文件ToolStripMenuItem.ShortcutKeys = ((System.Windows.Forms.Keys)((System.Windows.Forms.Keys.Control | System.Windows.Forms.Keys.O)));
            this.打开文件ToolStripMenuItem.Size = new System.Drawing.Size(171, 22);
            this.打开文件ToolStripMenuItem.Text = "打开文件";
            this.打开文件ToolStripMenuItem.Click += new System.EventHandler(this.打开文件ToolStripMenuItem_Click);
            // 
            // gaopai4tonghua
            // 
            this.gaopai4tonghua.AutoSize = true;
            this.gaopai4tonghua.Location = new System.Drawing.Point(236, 82);
            this.gaopai4tonghua.Name = "gaopai4tonghua";
            this.gaopai4tonghua.Size = new System.Drawing.Size(77, 16);
            this.gaopai4tonghua.TabIndex = 10;
            this.gaopai4tonghua.TabStop = true;
            this.gaopai4tonghua.Text = "高牌4同花";
            this.gaopai4tonghua.UseVisualStyleBackColor = true;
            // 
            // gaopai4shunzi
            // 
            this.gaopai4shunzi.AutoSize = true;
            this.gaopai4shunzi.Location = new System.Drawing.Point(236, 115);
            this.gaopai4shunzi.Name = "gaopai4shunzi";
            this.gaopai4shunzi.Size = new System.Drawing.Size(77, 16);
            this.gaopai4shunzi.TabIndex = 11;
            this.gaopai4shunzi.TabStop = true;
            this.gaopai4shunzi.Text = "高牌4顺子";
            this.gaopai4shunzi.UseVisualStyleBackColor = true;
            // 
            // infotext
            // 
            this.infotext.AutoSize = true;
            this.infotext.Font = new System.Drawing.Font("宋体", 10F);
            this.infotext.ForeColor = System.Drawing.Color.FromArgb(((int)(((byte)(0)))), ((int)(((byte)(0)))), ((int)(((byte)(192)))));
            this.infotext.Location = new System.Drawing.Point(12, 464);
            this.infotext.Name = "infotext";
            this.infotext.Size = new System.Drawing.Size(63, 14);
            this.infotext.TabIndex = 15;
            this.infotext.Text = "帮助信息";
            // 
            // v0
            // 
            this.v0.AutoSize = true;
            this.v0.Location = new System.Drawing.Point(282, 109);
            this.v0.Name = "v0";
            this.v0.Size = new System.Drawing.Size(29, 16);
            this.v0.TabIndex = 14;
            this.v0.TabStop = true;
            this.v0.Text = "0";
            this.v0.UseVisualStyleBackColor = true;
            // 
            // 帮助ToolStripMenuItem
            // 
            this.帮助ToolStripMenuItem.Name = "帮助ToolStripMenuItem";
            this.帮助ToolStripMenuItem.Size = new System.Drawing.Size(44, 21);
            this.帮助ToolStripMenuItem.Text = "帮助";
            this.帮助ToolStripMenuItem.Click += new System.EventHandler(this.帮助ToolStripMenuItem_Click);
            // 
            // Form11
            // 
            this.AllowDrop = true;
            this.AutoScaleDimensions = new System.Drawing.SizeF(6F, 12F);
            this.AutoScaleMode = System.Windows.Forms.AutoScaleMode.Font;
            this.ClientSize = new System.Drawing.Size(1184, 509);
            this.Controls.Add(this.infotext);
            this.Controls.Add(this.addcontribute);
            this.Controls.Add(this.addbettimes);
            this.Controls.Add(this.addcardfacevlue);
            this.Controls.Add(this.addcardtype);
            this.Controls.Add(this.btnRemoveNode);
            this.Controls.Add(this.addNodeBtn);
            this.Controls.Add(this.desclabel);
            this.Controls.Add(this.treeView1);
            this.Controls.Add(this.menuStrip1);
            this.MainMenuStrip = this.menuStrip1;
            this.Name = "Form11";
            this.Text = "empty";
            this.Load += new System.EventHandler(this.Form1_Load);
            this.DragDrop += new System.Windows.Forms.DragEventHandler(this.Form11_DragDrop);
            this.DragEnter += new System.Windows.Forms.DragEventHandler(this.Form11_DragEnter_1);
            this.addcardtype.ResumeLayout(false);
            this.addcardtype.PerformLayout();
            this.addcardfacevlue.ResumeLayout(false);
            this.addcardfacevlue.PerformLayout();
            this.addcontribute.ResumeLayout(false);
            this.addcontribute.PerformLayout();
            this.addbettimes.ResumeLayout(false);
            this.addbettimes.PerformLayout();
            ((System.ComponentModel.ISupportInitialize)(this.pass)).EndInit();
            ((System.ComponentModel.ISupportInitialize)(this.follow)).EndInit();
            ((System.ComponentModel.ISupportInitialize)(this.all)).EndInit();
            ((System.ComponentModel.ISupportInitialize)(this.add)).EndInit();
            ((System.ComponentModel.ISupportInitialize)(this.addlowlimit)).EndInit();
            ((System.ComponentModel.ISupportInitialize)(this.addtoplimit)).EndInit();
            ((System.ComponentModel.ISupportInitialize)(this.bettimes)).EndInit();
            this.menuStrip1.ResumeLayout(false);
            this.menuStrip1.PerformLayout();
            this.ResumeLayout(false);
            this.PerformLayout();

        }

        #endregion

        private System.Windows.Forms.TreeView treeView1;
        private System.Windows.Forms.Panel addcardtype;
        private System.Windows.Forms.RadioButton yidui;
        private System.Windows.Forms.RadioButton gaopai;
        private System.Windows.Forms.RadioButton tonghuashun;
        private System.Windows.Forms.RadioButton tonghua;
        private System.Windows.Forms.RadioButton shunzi;
        private System.Windows.Forms.RadioButton liangdui;
        private System.Windows.Forms.RadioButton santiao;
        private System.Windows.Forms.RadioButton hulu;
        private System.Windows.Forms.RadioButton sitiao;
        private System.Windows.Forms.Button addNodeBtn;
        private System.Windows.Forms.Panel addcardfacevlue;
        private System.Windows.Forms.RadioButton v3;
        private System.Windows.Forms.RadioButton v4;
        private System.Windows.Forms.RadioButton v5;
        private System.Windows.Forms.RadioButton v6;
        private System.Windows.Forms.RadioButton v7;
        private System.Windows.Forms.RadioButton v8;
        private System.Windows.Forms.RadioButton v9;
        private System.Windows.Forms.RadioButton v10;
        private System.Windows.Forms.RadioButton v11;
        private System.Windows.Forms.RadioButton v12;
        private System.Windows.Forms.RadioButton v13;
        private System.Windows.Forms.RadioButton v14;
        private System.Windows.Forms.RadioButton v2;
        private System.Windows.Forms.Label desclabel;
        private System.Windows.Forms.Button btnRemoveNode;
        private System.Windows.Forms.Panel addcontribute;
        private System.Windows.Forms.RadioButton con2;
        private System.Windows.Forms.RadioButton con1;
        private System.Windows.Forms.RadioButton con0;
        private System.Windows.Forms.Panel addbettimes;
        private System.Windows.Forms.Label label2;
        private System.Windows.Forms.Label label4;
        private System.Windows.Forms.Label label3;
        private System.Windows.Forms.Label label8;
        private System.Windows.Forms.Label label7;
        private System.Windows.Forms.Label label6;
        private System.Windows.Forms.Label label5;
        private System.Windows.Forms.NumericUpDown bettimes;
        private System.Windows.Forms.NumericUpDown pass;
        private System.Windows.Forms.NumericUpDown follow;
        private System.Windows.Forms.NumericUpDown all;
        private System.Windows.Forms.NumericUpDown add;
        private System.Windows.Forms.NumericUpDown addlowlimit;
        private System.Windows.Forms.NumericUpDown addtoplimit;
        private System.Windows.Forms.MenuStrip menuStrip1;
        private System.Windows.Forms.ToolStripMenuItem 新建文档ToolStripMenuItem;
        private System.Windows.Forms.ToolStripMenuItem 新建文档ToolStripMenuItem1;
        private System.Windows.Forms.ToolStripMenuItem 保存ToolStripMenuItem;
        private System.Windows.Forms.ToolStripMenuItem 另存为ToolStripMenuItem;
        private System.Windows.Forms.Button button3;
        private System.Windows.Forms.Label leftWeight;
        private System.Windows.Forms.Button button1;
        private System.Windows.Forms.ToolStripMenuItem 打开文件ToolStripMenuItem;
        private System.Windows.Forms.RadioButton gaopai4shunzi;
        private System.Windows.Forms.RadioButton gaopai4tonghua;
        private System.Windows.Forms.Label infotext;
        private System.Windows.Forms.RadioButton v0;
        private System.Windows.Forms.ToolStripMenuItem 帮助ToolStripMenuItem;

    }
}

